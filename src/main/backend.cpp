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

namespace lsp
{
    namespace r3d
    {
        namespace wgl
        {
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

//            static GLint rgba24x32[]    = { GLX_RGBA, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8, GLX_DEPTH_SIZE, 32, GLX_DOUBLEBUFFER, None };
//            static GLint rgba24x24[]    = { GLX_RGBA, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
//            static GLint rgba16x24[]    = { GLX_RGBA, GLX_RED_SIZE, 5, GLX_GREEN_SIZE, 6, GLX_BLUE_SIZE, 5, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
//            static GLint rgba15x24[]    = { GLX_RGBA, GLX_RED_SIZE, 5, GLX_GREEN_SIZE, 5, GLX_BLUE_SIZE, 5, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
//            static GLint rgba16[]       = { GLX_RGBA, GLX_RED_SIZE, 5, GLX_GREEN_SIZE, 6, GLX_BLUE_SIZE, 5, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
//            static GLint rgba15[]       = { GLX_RGBA, GLX_RED_SIZE, 5, GLX_GREEN_SIZE, 5, GLX_BLUE_SIZE, 5, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
//            static GLint rgbax32[]      = { GLX_RGBA, GLX_DEPTH_SIZE, 32, GLX_DOUBLEBUFFER, None };
//            static GLint rgbax24[]      = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
//            static GLint rgbax16[]      = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
//            static GLint rgba[]         = { GLX_RGBA, GLX_DOUBLEBUFFER, None };

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

                // Destroy the context and the window
                if (_this->hDC != NULL)
                {
                    if (_this->hGL != NULL)
                    {
                        if (wglGetCurrentContext() == _this->hGL)
                            wglMakeCurrent(_this->hDC, NULL);
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
                    free(_this->pWndClass);
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
                _this->hGL      = wglCreateContext(_this->hDC);
                if (_this->hGL == NULL)
                {
                    lsp_error("Error creating context: code=%ld", long(GetLastError()));
                    return STATUS_UNKNOWN_ERR;
                }

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

                return STATUS_OK;
            }

            status_t backend_t::start(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);
                if (_this->hGL == NULL)
                    return STATUS_BAD_STATE;

                // Set active context
                wglMakeCurrent(_this->hDC, _this->hGL);

                return STATUS_OK;
            }

            status_t backend_t::set_matrix(r3d::backend_t *handle, r3d::matrix_type_t type, const r3d::mat4_t *m)
            {
                return r3d::base_backend_t::set_matrix(handle, type, m);
            }

            status_t backend_t::set_lights(r3d::backend_t *handle, const r3d::light_t *lights, size_t count)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                return STATUS_OK;
            }

            status_t backend_t::draw_primitives(r3d::backend_t *handle, const r3d::buffer_t *buffer)
            {
                backend_t *_this = static_cast<backend_t *>(handle);


                return STATUS_OK;
            }

            status_t backend_t::sync(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                return STATUS_OK;
            }

            status_t backend_t::read_pixels(r3d::backend_t *handle, void *buf, size_t stride, r3d::pixel_format_t format)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                return STATUS_OK;
            }

            status_t backend_t::finish(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);
                if (_this->hGL == NULL)
                    return STATUS_BAD_STATE;

                // Set active context
                if (wglGetCurrentContext() == _this->hGL)
                    wglMakeCurrent(_this->hDC, NULL);

                return STATUS_OK;
            }
        } /* namespace wgl */
    } /* namespace r3d */
} /* namespace lsp */

