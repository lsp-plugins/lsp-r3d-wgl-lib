/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-r3d-wgl-lib
 * Created on: 24 апр. 2019 г.
 *
 * lsp-r3d-wgl-lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-r3d-wgl-lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-r3d-wgl-lib. If not, see <https://www.gnu.org/licenses/>.
 */

#include <lsp-plug.in/common/types.h>
#include <lsp-plug.in/common/debug.h>
#include <lsp-plug.in/r3d/wgl/backend.h>

#include <stdlib.h>
#include <shlwapi.h>
#include <wchar.h>
#include <gl/gl.h>
#include <gl/glext.h>

namespace lsp
{
    namespace r3d
    {
        namespace wgl
        {
            enum buffer_state_t
            {
                DBUF_VINDEX  = 1 << 0,
                DBUF_NORMAL  = 1 << 1,
                DBUF_NINDEX  = 1 << 2,
                DBUF_COLOR   = 1 << 3,
                DBUF_CINDEX  = 1 << 4,

                DBUF_NORMAL_FLAGS = DBUF_NORMAL | DBUF_NINDEX,
                DBUF_COLOR_FLAGS  = DBUF_COLOR  | DBUF_CINDEX,
                DBUF_INDEX_MASK   = DBUF_VINDEX | DBUF_NINDEX | DBUF_CINDEX
            };

            constexpr size_t VATTR_BUFFER_SIZE      = 3072;    // Multiple of 3

        #define PFD(color_bits, r_bits, g_bits, b_bits, a_bits, depth_bits) \
            { \
                /* nSize */ sizeof(PIXELFORMATDESCRIPTOR), \
                /* nVersion */ 1, \
                /* dwFlags */ PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, \
                /* iPixelType */ PFD_TYPE_RGBA, \
                /* cColorBits */ color_bits, \
                /* cRedBits */ r_bits, \
                /* cRedShift */ 0, \
                /* cGreenBits */ g_bits, \
                /* cGreenShift */ 0, \
                /* cBlueBits */ b_bits, \
                /* cBlueShift */ 0, \
                /* cAlphaBits */ a_bits, \
                /* cAlphaShift */ 0, \
                /* cAccumBits */ 0, \
                /* cAccumRedBits */ 0, \
                /* cAccumGreenBits */ 0, \
                /* cAccumBlueBits */ 0, \
                /* cAccumAlphaBits */ 0, \
                /* cDepthBits */ depth_bits, \
                /* cStencilBits */ 0, \
                /* cAuxBuffers */ 0, \
                /* iLayerType */ PFD_MAIN_PLANE, \
                /* bReserved */ 0, \
                /* dwLayerMask */ 0, \
                /* dwVisibleMask */ 0, \
                /* dwDamageMask */ 0 \
            }

            static const PIXELFORMATDESCRIPTOR pixel_formats[] = {
                PFD(24, 8, 8, 8, 8, 32),
                PFD(24, 8, 8, 8, 8, 24),
                PFD(16, 5, 6, 5, 0, 24),
                PFD(15, 5, 5, 5, 0, 24),
                PFD(16, 5, 6, 5, 0, 16),
                PFD(15, 5, 5, 5, 0, 16),

                PFD(32, 0, 0, 0, 0, 32),
                PFD(32, 0, 0, 0, 0, 24),
                PFD(32, 0, 0, 0, 0, 16),
                PFD(24, 0, 0, 0, 0, 32),
                PFD(24, 0, 0, 0, 0, 24),
                PFD(24, 0, 0, 0, 0, 16),
                PFD(16, 0, 0, 0, 0, 32),
                PFD(16, 0, 0, 0, 0, 24),
                PFD(16, 0, 0, 0, 0, 16)
            };
        #undef PFD

            backend_t::backend_t()
            {
                construct();
            }

            static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
            {
                switch (uMsg)
                {
                    case WM_CREATE:
                        CREATESTRUCTW *create = reinterpret_cast<CREATESTRUCTW *>(lParam);
                        backend_t *wnd = reinterpret_cast<backend_t *>(create->lpCreateParams);
                        if (wnd != NULL)
                            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
                        return DefWindowProc(hwnd, uMsg, wParam, lParam);
                }

                return DefWindowProcW(hwnd, uMsg, wParam, lParam);
            }

            void backend_t::construct()
            {
                pWndClass       = NULL;
                hWindow         = NULL;
                hDC             = NULL;
                hGL             = NULL;
                bDrawing        = false;
                vxBuffer        = NULL;

                base_backend_t::construct();

                // Export virtual table
                #define R3D_WGL_BACKEND_EXP(func)   r3d::backend_t::func = backend_t::func;
                R3D_WGL_BACKEND_EXP(init_window);
                R3D_WGL_BACKEND_EXP(init_offscreen);
                R3D_WGL_BACKEND_EXP(destroy);
                R3D_WGL_BACKEND_EXP(locate);

                R3D_WGL_BACKEND_EXP(start);
                R3D_WGL_BACKEND_EXP(sync);
                R3D_WGL_BACKEND_EXP(read_pixels);
                R3D_WGL_BACKEND_EXP(finish);

                R3D_WGL_BACKEND_EXP(set_matrix);
                R3D_WGL_BACKEND_EXP(set_lights);
                R3D_WGL_BACKEND_EXP(draw_primitives);

                #undef R3D_GLX_BACKEND_EXP
            }

            void backend_t::destroy(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                // Destroy vertex attributes buffer
                if (_this->vxBuffer != NULL)
                {
                    free(_this->vxBuffer);
                    _this->vxBuffer     = NULL;
                }

                // Destroy the context and the window
                if (_this->hDC != NULL)
                {
                    if (_this->hGL != NULL)
                    {
                        if (::wglGetCurrentContext() == _this->hGL)
                            ::wglMakeCurrent(_this->hDC, NULL);
                    }
                    _this->hDC          = NULL;
                }
                if (_this->hGL != NULL)
                {
                    wglDeleteContext(_this->hGL);
                    _this->hGL          = NULL;
                }
                if (_this->hWindow != NULL)
                {
                    DestroyWindow(_this->hWindow);
                    _this->hWindow      = NULL;
                }
                if (_this->pWndClass != NULL)
                {
                    UnregisterClassW(_this->pWndClass, GetModuleHandleW(NULL));
                    //free(_this->pWndClass); // SIGTRAP from windows internals
                    _this->pWndClass    = NULL;
                }

                // Call parent structure for destroy
                r3d::base_backend_t::destroy(handle);
            }

            status_t backend_t::init_window(r3d::backend_t *handle, void **out_window)
            {
                backend_t *_this = static_cast<backend_t *>(handle);
                if (_this->hWindow != NULL)
                    return STATUS_BAD_STATE;

                if (_this->pWndClass == NULL)
                {
                    // Generate class name of the window
                    WCHAR class_name[80];
                    swprintf_s(class_name, sizeof(class_name), L"lsp-wgl-%p", _this);
                    if ((_this->pWndClass = StrDupW(class_name)) == NULL)
                        return STATUS_NO_MEM;

                    // Register window class name
                    WNDCLASSW wc;
                    ZeroMemory(&wc, sizeof(wc));

                    wc.style         = CS_HREDRAW | CS_VREDRAW;
                    wc.lpfnWndProc   = window_proc;
                    wc.hInstance     = GetModuleHandleW(NULL);
                    wc.lpszClassName = _this->pWndClass;

                    if (!RegisterClassW(&wc))
                        return STATUS_UNKNOWN_ERR;
                }

                // Create window
                _this->hWindow = CreateWindowExW(
                    0,                                  // dwExStyle
                    _this->pWndClass,                   // lpClassName
                    L"WGL Offscreen Window",            // lpWindowName
                    WS_OVERLAPPEDWINDOW,                // dwStyle
                    0,                                  // X
                    0,                                  // Y
                    1,                                  // nWidth
                    1,                                  // nHeight
                    NULL,                               // hWndParent
                    NULL,                               // hMenu
                    GetModuleHandleW(NULL),             // hInstance
                    _this);                             // lpCreateParam
                if (_this->hWindow == NULL)
                    return STATUS_UNKNOWN_ERR;

                // Get device context
                _this->hDC      = GetDC(_this->hWindow);
                if (_this->hDC == NULL)
                    return STATUS_UNKNOWN_ERR;

                // Choose pixel format
                for (size_t i=0; i < sizeof(pixel_formats) / sizeof(PIXELFORMATDESCRIPTOR); ++i)
                {
                    int pixel_fmt = ChoosePixelFormat(_this->hDC, &pixel_formats[i]);
                    if (pixel_fmt != 0)
                    {
                        PIXELFORMATDESCRIPTOR pfd;
                        ZeroMemory(&pfd, sizeof(pfd));
                        pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
                        if (DescribePixelFormat(_this->hDC, pixel_fmt, sizeof(pfd), &pfd) != 0)
                        {
                            lsp_trace("Selected pixel format: %d BPP (r:%d g:%d b:%d a:%d) @ %d depth",
                                int(pfd.cColorBits),
                                int(pfd.cRedBits), int(pfd.cGreenBits), int(pfd.cBlueBits), int(pfd.cAlphaBits),
                                int(pfd.cDepthBits));
                        }

                        SetPixelFormat(_this->hDC, pixel_fmt, &pixel_formats[i]);
                        break;
                    }
                }

                // Create OpenGL context
                _this->hGL      = ::wglCreateContext(_this->hDC);
                if (_this->hGL == NULL)
                {
                    lsp_error("Error creating context: code=%ld", long(GetLastError()));
                    return STATUS_UNKNOWN_ERR;
                }

//                ShowWindow(_this->hWindow, SW_SHOWNORMAL);

                return STATUS_OK;
            }

            status_t backend_t::init_offscreen(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);
                void *hwnd = NULL;

                return _this->init_window(handle, &hwnd);
            }

            status_t backend_t::locate(r3d::backend_t *handle, ssize_t left, ssize_t top, ssize_t width, ssize_t height)
            {
                backend_t *_this = static_cast<backend_t *>(handle);
                if ((_this->hGL == NULL) || (_this->bDrawing))
                    return STATUS_BAD_STATE;

                ::glViewport(0, 0, width, height);

                if ((_this->viewLeft == left) &&
                    (_this->viewTop == top) &&
                    (_this->viewWidth == width) &&
                    (_this->viewHeight == height))
                    return STATUS_OK;

                RECT rect;
                MoveWindow(_this->hWindow, left, top, width, height, FALSE);
                GetClientRect(_this->hWindow, &rect);
                MoveWindow(
                    _this->hWindow,
                    left,
                    top,
                    width*2  - (rect.right  - rect.left),
                    height*2 - (rect.bottom - rect.top),
                    FALSE);

                // Update parameters
                _this->viewLeft    = left;
                _this->viewTop     = top;
                _this->viewWidth   = width;
                _this->viewHeight  = height;

                return STATUS_OK;
            }

            status_t backend_t::start(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);
                if ((_this->hGL == NULL) || (_this->bDrawing))
                    return STATUS_BAD_STATE;

                // Set active context
                ::wglMakeCurrent(_this->hDC, _this->hGL);
                ::glViewport(0, 0, _this->viewWidth, _this->viewHeight);
                ::glDrawBuffer(GL_BACK);

                // Enable depth test and culling
                ::glDepthFunc(GL_LEQUAL);
                ::glEnable(GL_DEPTH_TEST);
                ::glEnable(GL_CULL_FACE);
                ::glCullFace(GL_BACK);
                ::glEnable(GL_COLOR_MATERIAL);

                // Tune lighting
                ::glShadeModel(GL_SMOOTH);
                ::glEnable(GL_RESCALE_NORMAL);

                // Special tuning for non-poligonal primitives
                ::glPolygonOffset(1.0f, 2.0f);
                ::glEnable(GL_POLYGON_OFFSET_POINT);
                ::glEnable(GL_POLYGON_OFFSET_FILL);
                ::glEnable(GL_POLYGON_OFFSET_LINE);

                // Clear buffer
                ::glClearColor(_this->colBackground.r, _this->colBackground.g, _this->colBackground.b, _this->colBackground.a);
                ::glClearDepth(1.0);
                ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Setup drawing flag
                _this->bDrawing     = true;

                return STATUS_OK;
            }

            status_t backend_t::set_matrix(r3d::backend_t *handle, r3d::matrix_type_t type, const r3d::mat4_t *m)
            {
                return r3d::base_backend_t::set_matrix(handle, type, m);
            }

            status_t backend_t::set_lights(r3d::backend_t *handle, const r3d::light_t *lights, size_t count)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                if ((_this->hDC == NULL) || (!_this->bDrawing))
                    return STATUS_BAD_STATE;

                // Enable all possible lights
                size_t light_id = GL_LIGHT0;

                ::glMatrixMode(GL_MODELVIEW);
                ::glPushMatrix();
                ::glLoadIdentity();

                for (size_t i=0; i<count; ++i)
                {
                    // Skip disabled lights
                    if (lights[i].type == r3d::LIGHT_NONE)
                        continue;

                    // Enable the light and set basic attributes
                    r3d::vec4_t position;

                    ::glEnable(light_id);
                    ::glLightfv(light_id, GL_AMBIENT, &lights[i].ambient.r);
                    ::glLightfv(light_id, GL_DIFFUSE, &lights[i].diffuse.r);
                    ::glLightfv(light_id, GL_SPECULAR, &lights[i].specular.r);

                    switch (lights[i].type)
                    {
                        case r3d::LIGHT_POINT:
                            position.dx     = lights[i].position.x;
                            position.dy     = lights[i].position.y;
                            position.dz     = lights[i].position.z;
                            position.dw     = 1.0f;
                            ::glLightfv(light_id, GL_POSITION, &position.dx);
                            ::glLighti(light_id, GL_SPOT_CUTOFF, 180);
                            break;
                        case r3d::LIGHT_DIRECTIONAL:
                            position.dx     = lights[i].direction.dx;
                            position.dy     = lights[i].direction.dy;
                            position.dz     = lights[i].direction.dz;
                            position.dw     = 0.0f;
                            ::glLightfv(light_id, GL_POSITION, &position.dx);
                            ::glLighti(light_id, GL_SPOT_CUTOFF, 180);
                            break;
                        case r3d::LIGHT_SPOT:
                            position.dx     = lights[i].position.x;
                            position.dy     = lights[i].position.y;
                            position.dz     = lights[i].position.z;
                            position.dw     = 1.0f;
                            ::glLightfv(light_id, GL_POSITION, &position.dx);
                            ::glLightfv(light_id, GL_SPOT_DIRECTION, &lights[i].direction.dx);
                            ::glLightf(light_id, GL_SPOT_CUTOFF, lights[i].cutoff);
                            ::glLightf(light_id, GL_CONSTANT_ATTENUATION, lights[i].constant);
                            ::glLightf(light_id, GL_LINEAR_ATTENUATION, lights[i].linear);
                            ::glLightf(light_id, GL_QUADRATIC_ATTENUATION, lights[i].quadratic);
                            break;
                        default:
                            return STATUS_INVALID_VALUE;
                    }

                    // Ignore all lights that are out of 8 basic lights
                    if (++light_id > GL_LIGHT7)
                        break;
                }

                // Disable all other non-related lights
                while (light_id <= GL_LIGHT7)
                    ::glDisable(light_id++);

                ::glPopMatrix();

                return STATUS_OK;
            }

            void gl_draw_arrays_simple(GLenum mode, size_t bstate, const r3d::buffer_t *buffer, size_t count)
            {
                // Enable vertex pointer (if present)
                ::glEnableClientState(GL_VERTEX_ARRAY);
                ::glVertexPointer(4, GL_FLOAT,
                    (buffer->vertex.stride == 0) ? sizeof(r3d::dot4_t) : buffer->vertex.stride,
                    buffer->vertex.data
                );

                // Enable normal pointer
                if (bstate & DBUF_NORMAL)
                {
                    ::glEnableClientState(GL_NORMAL_ARRAY);
                    ::glNormalPointer(GL_FLOAT,
                        (buffer->normal.stride == 0) ? sizeof(r3d::vec4_t) : buffer->normal.stride,
                        buffer->normal.data
                    );
                }
                else
                    ::glDisableClientState(GL_NORMAL_ARRAY);

                // Enable color pointer
                if (bstate & DBUF_COLOR)
                {
                    ::glEnableClientState(GL_COLOR_ARRAY);
                    ::glColorPointer(4, GL_FLOAT,
                        (buffer->color.stride == 0) ? sizeof(r3d::color_t) : buffer->color.stride,
                        buffer->color.data
                    );
                }
                else
                {
                    ::glColor4fv(&buffer->color.dfl.r);         // Set-up default color
                    ::glDisableClientState(GL_COLOR_ARRAY);
                }

                // Draw the elements (or arrays, depending on configuration)
                if (buffer->type != r3d::PRIMITIVE_WIREFRAME_TRIANGLES)
                {
                    if (bstate & DBUF_VINDEX)
                        ::glDrawElements(mode, count, GL_UNSIGNED_INT, buffer->vertex.index);
                    else
                        ::glDrawArrays(mode, 0, count);
                }
                else
                {
                    if (bstate & DBUF_VINDEX)
                    {
                        const uint32_t *ptr = buffer->vertex.index;
                        for (size_t i=0; i<count; i += 3, ptr += 3)
                            ::glDrawElements(mode, 3, GL_UNSIGNED_INT, ptr);
                    }
                    else
                    {
                        for (size_t i=0; i<count; i += 3)
                            ::glDrawArrays(mode, i, 3);
                    }
                }

                // Disable previous settings
                if (bstate & DBUF_COLOR)
                    ::glDisableClientState(GL_COLOR_ARRAY);
                if (bstate & DBUF_NORMAL)
                    ::glDisableClientState(GL_NORMAL_ARRAY);
                ::glDisableClientState(GL_VERTEX_ARRAY);
            }

            void gl_draw_arrays_indexed(r3d::backend_t *handle, GLenum mode, size_t bstate, const r3d::buffer_t *buffer, size_t count)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                // Lazy initialization: allocate temporary buffer
                if (_this->vxBuffer == NULL)
                {
                    _this->vxBuffer = reinterpret_cast<vertex_t *>(malloc(VATTR_BUFFER_SIZE * sizeof(vertex_t)));
                    if (_this->vxBuffer == NULL)
                        return;
                }

                // Enable vertex pointer
                ::glEnableClientState(GL_VERTEX_ARRAY);
                ::glVertexPointer(4, GL_FLOAT, sizeof(vertex_t), &_this->vxBuffer->v);

                // Enable normal pointer
                if (bstate & DBUF_NORMAL)
                {
                    ::glEnableClientState(GL_NORMAL_ARRAY);
                    ::glNormalPointer(GL_FLOAT, sizeof(vertex_t), &_this->vxBuffer->n);
                }
                else
                    ::glDisableClientState(GL_NORMAL_ARRAY);

                // Enable color pointer
                if (bstate & DBUF_COLOR)
                {
                    ::glEnableClientState(GL_COLOR_ARRAY);
                    ::glColorPointer(4, GL_FLOAT, sizeof(vertex_t), &_this->vxBuffer->c);
                }
                else
                {
                    ::glColor4fv(&buffer->color.dfl.r);         // Set-up default color
                    ::glDisableClientState(GL_COLOR_ARRAY);
                }

                // Compute stride values and indices
                const uint32_t *vindex  = buffer->vertex.index;
                const uint32_t *nindex  = buffer->normal.index;
                const uint32_t *cindex  = buffer->color.index;
                const uint8_t  *vbuf    = reinterpret_cast<const uint8_t *>(buffer->vertex.data);
                const uint8_t  *nbuf    = reinterpret_cast<const uint8_t *>(buffer->normal.data);
                const uint8_t  *cbuf    = reinterpret_cast<const uint8_t *>(buffer->color.data);
                size_t vstride          = (buffer->vertex.stride == 0) ? sizeof(r3d::dot4_t)  : buffer->vertex.stride;
                size_t nstride          = (buffer->normal.stride == 0) ? sizeof(r3d::vec4_t)  : buffer->normal.stride;
                size_t cstride          = (buffer->color.stride == 0)  ? sizeof(r3d::color_t) : buffer->color.stride;

                for (size_t off = 0; off < count; )
                {
                    size_t to_do    = count - off;
                    if (to_do > VATTR_BUFFER_SIZE)
                        to_do           = VATTR_BUFFER_SIZE;

                    // Fill the temporary buffer data
                    vertex_t *vx    = _this->vxBuffer;
                    for (size_t i=0; i<to_do; ++i, ++vx)
                    {
                        size_t vxi      = off + i;

                        // Add vertex coordinates
                        if (bstate & DBUF_VINDEX)
                            vx->v       = *(reinterpret_cast<const dot4_t *>(&vbuf[vindex[vxi] * vstride]));
                        else
                            vx->v       = *(reinterpret_cast<const dot4_t *>(&vbuf[vxi * vstride]));

                        // Add normal coordinates if present
                        if (bstate & DBUF_NORMAL)
                        {
                            if (bstate & DBUF_NINDEX)
                                vx->n       = *(reinterpret_cast<const vec4_t *>(&nbuf[nindex[vxi] * nstride]));
                            else
                                vx->n       = *(reinterpret_cast<const vec4_t *>(&nbuf[vxi * nstride]));
                        }

                        // Add color coordinates if present
                        if (bstate & DBUF_COLOR)
                        {
                            if (bstate & DBUF_CINDEX)
                                vx->c       = *(reinterpret_cast<const color_t *>(&cbuf[cindex[vxi] * cstride]));
                            else
                                vx->c       = *(reinterpret_cast<const color_t *>(&cbuf[vxi * cstride]));
                        }
                    }

                    // Draw the buffer
                    if (buffer->type != r3d::PRIMITIVE_WIREFRAME_TRIANGLES)
                        ::glDrawArrays(mode, 0, count);
                    else
                    {
                        for (size_t i=0; i<count; i += 3)
                            ::glDrawArrays(mode, i, 3);
                    }

                    // Update offset
                    off            += to_do;
                }

                // Disable previous settings
                if (bstate & DBUF_COLOR)
                    ::glDisableClientState(GL_COLOR_ARRAY);
                if (bstate & DBUF_NORMAL)
                    ::glDisableClientState(GL_NORMAL_ARRAY);
                ::glDisableClientState(GL_VERTEX_ARRAY);
            }

            status_t backend_t::draw_primitives(r3d::backend_t *handle, const r3d::buffer_t *buffer)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                if (buffer == NULL)
                    return STATUS_BAD_ARGUMENTS;
                if ((_this->hDC == NULL) || (!_this->bDrawing))
                    return STATUS_BAD_STATE;

                // Is there any data to draw?
                if (buffer->count <= 0)
                    return STATUS_OK;

                //-------------------------------------------------------------
                // Select the drawing mode

                // Check primitive type to draw
                GLenum mode  = GL_TRIANGLES;
                size_t count = buffer->count;

                switch (buffer->type)
                {
                    case r3d::PRIMITIVE_TRIANGLES:
                        mode    = GL_TRIANGLES;
                        count   = (count << 1) + count; // count *= 3
                        break;
                    case r3d::PRIMITIVE_WIREFRAME_TRIANGLES:
                        mode    = GL_LINE_LOOP;
                        count   = (count << 1) + count; // count *= 3
                        ::glLineWidth(buffer->width);
                        break;
                    case r3d::PRIMITIVE_LINES:
                        mode    = GL_LINES;
                        count <<= 1;                    // count *= 2
                        ::glLineWidth(buffer->width);
                        break;
                    case r3d::PRIMITIVE_POINTS:
                        mode    = GL_POINTS;
                        ::glPointSize(buffer->width);
                        break;
                    default:
                        return STATUS_BAD_ARGUMENTS;
                }

                size_t bstate = 0;
                if (buffer->vertex.data == NULL)
                    return STATUS_BAD_ARGUMENTS;
                if (buffer->vertex.index != NULL)
                    bstate     |= DBUF_VINDEX;

                if (buffer->normal.data != NULL)
                    bstate     |= DBUF_NORMAL;
                if (buffer->normal.index != NULL)
                    bstate     |= DBUF_NINDEX;

                if (buffer->color.data != NULL)
                    bstate     |= DBUF_COLOR;
                if (buffer->color.index != NULL)
                    bstate     |= DBUF_CINDEX;

                if (((bstate & DBUF_NORMAL_FLAGS) == DBUF_NINDEX) ||
                    ((bstate & DBUF_COLOR_FLAGS) == DBUF_CINDEX))
                    return STATUS_BAD_ARGUMENTS; // Index buffers can not be definde without data buffers

                //-------------------------------------------------------------
                // Prepare drawing state
                // Load matrices
                ::glMatrixMode(GL_PROJECTION);
                ::glLoadMatrixf(_this->matProjection.m);
                ::glMatrixMode(GL_MODELVIEW);
                ::glLoadMatrixf(_this->matView.m);
                ::glMultMatrixf(_this->matWorld.m);
                ::glMultMatrixf(buffer->model.m);

                // enable blending
                if (buffer->flags & r3d::BUFFER_BLENDING)
                {
                    ::glEnable(GL_BLEND);
                    if (buffer->flags & r3d::BUFFER_STD_BLENDING)
                        ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    else
                        ::glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
                }
                if (buffer->flags & r3d::BUFFER_LIGHTING)
                    ::glEnable(GL_LIGHTING);
                if (buffer->flags & r3d::BUFFER_NO_CULLING)
                    ::glDisable(GL_CULL_FACE);

                //-------------------------------------------------------------
                // Draw the buffer data
                if (!(bstate & (DBUF_NINDEX | DBUF_CINDEX)))
                    gl_draw_arrays_simple(mode, bstate, buffer, count);
                else
                    gl_draw_arrays_indexed(handle, mode, bstate, buffer, count);

                //-------------------------------------------------------------
                // Reset the drawing state
                if (buffer->flags & r3d::BUFFER_BLENDING)
                    ::glDisable(GL_BLEND);
                if (buffer->flags & r3d::BUFFER_LIGHTING)
                    ::glDisable(GL_LIGHTING);
                if (buffer->flags & r3d::BUFFER_NO_CULLING)
                    ::glEnable(GL_CULL_FACE);

                return STATUS_OK;
            }

            status_t backend_t::sync(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                if ((_this->hGL == NULL) || (!_this->bDrawing))
                    return STATUS_BAD_STATE;

                ::glFinish();
                ::glFlush();

                return STATUS_OK;
            }

            status_t backend_t::read_pixels(r3d::backend_t *handle, void *buf, size_t stride, r3d::pixel_format_t format)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                if ((_this->hDC == NULL) || (!_this->bDrawing))
                    return STATUS_BAD_STATE;

                size_t fmt;
                switch (format)
                {
                    case r3d::PIXEL_RGBA:   fmt     = GL_RGBA;      break;
                    case r3d::PIXEL_BGRA:   fmt     = GL_BGRA_EXT;  break;
                    case r3d::PIXEL_RGB:    fmt     = GL_RGB;       break;
                    case r3d::PIXEL_BGR:    fmt     = GL_BGR_EXT;   break;
                    default:
                        return STATUS_BAD_ARGUMENTS;
                }

                ::glFinish();
                ::glFlush();
                ::glReadBuffer(GL_BACK);

                uint8_t *ptr = reinterpret_cast<uint8_t *>(buf);
                for (ssize_t i=0; i<_this->viewHeight; ++i)
                {
                    ssize_t row  = _this->viewHeight - i - 1;
                    ::glGetError();
                    ::glReadPixels(0, row, _this->viewWidth, 1, fmt, GL_UNSIGNED_BYTE, ptr);
                    GLenum err = ::glGetError();
                    if (err != GL_NO_ERROR)
                        lsp_trace("debug");
                    ptr     += stride;
                }

                return STATUS_OK;
            }

            status_t backend_t::finish(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);
                if ((_this->hGL == NULL) || (!_this->bDrawing))
                    return STATUS_BAD_STATE;

                ::glFinish();
                ::glFlush();
                SwapBuffers(_this->hDC);

                // Set active context
                if (::wglGetCurrentContext() == _this->hGL)
                    ::wglMakeCurrent(_this->hDC, NULL);

                // Reset drawing flag
                _this->bDrawing     = false;

                return STATUS_OK;
            }
        } /* namespace wgl */
    } /* namespace r3d */
} /* namespace lsp */

