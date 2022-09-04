/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-r3d-wgl-lib
 * Created on: 18 апр. 2019 г.
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

#ifndef LSP_PLUG_IN_R3D_WGL_BACKEND_H_
#define LSP_PLUG_IN_R3D_WGL_BACKEND_H_

#include <lsp-plug.in/r3d/wgl/version.h>

#include <lsp-plug.in/r3d/base/backend.h>

#include <gl/gl.h>
#include <windows.h>

namespace lsp
{
    namespace r3d
    {
        namespace wgl
        {
            typedef struct vertex_t
            {
                dot4_t          v;      // Vertex
                vec4_t          n;      // Normal
                color_t         c;      // Color
            } vertex_t;

            typedef struct backend_t: public r3d::base_backend_t
            {
                WCHAR              *pWndClass;      // Window class
                HWND                hWindow;        // Window instance
                HDC                 hDC;            // Device context instance
                HGLRC               hGL;            // OpenGL context instance
                bool                bDrawing;       // Flag: backend is in drawing mode
                vertex_t           *vxBuffer;       // Temporary vertex buffer

                void                construct();
                explicit            backend_t();

                static void         destroy(r3d::backend_t *handle);
                static status_t     init_window(r3d::backend_t *handle, void **out_window);
                static status_t     init_offscreen(r3d::backend_t *handle);

                static status_t     locate(r3d::backend_t *handle, ssize_t left, ssize_t top, ssize_t width, ssize_t height);
                static status_t     start(r3d::backend_t *handle);
                static status_t     set_matrix(r3d::backend_t *handle, r3d::matrix_type_t type, const r3d::mat4_t *m);
                static status_t     set_lights(r3d::backend_t *handle, const r3d::light_t *lights, size_t count);
                static status_t     draw_primitives(r3d::backend_t *handle, const r3d::buffer_t *buffer);
                static status_t     sync(r3d::backend_t *handle);
                static status_t     read_pixels(r3d::backend_t *handle, void *buf, r3d::pixel_format_t format);
                static status_t     finish(r3d::backend_t *handle);

            } backend_t;

        } /* namespace wgl */
    } /* namespace r3d */
} /* namespace lsp */


#endif /* LSP_PLUG_IN_R3D_WGL_BACKEND_H_ */
