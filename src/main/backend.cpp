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

namespace lsp
{
    namespace r3d
    {
        namespace wgl
        {
            backend_t::backend_t()
            {
                construct();
            }

            void backend_t::construct()
            {
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

                // Call parent structure for destroy
                r3d::base_backend_t::destroy(handle);
            }

            status_t backend_t::init_window(r3d::backend_t *handle, void **out_window)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                return STATUS_OK;
            }

            status_t backend_t::init_offscreen(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                return STATUS_OK;
            }

            status_t backend_t::locate(r3d::backend_t *handle, ssize_t left, ssize_t top, ssize_t width, ssize_t height)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

                return STATUS_OK;
            }

            status_t backend_t::start(r3d::backend_t *handle)
            {
                backend_t *_this = static_cast<backend_t *>(handle);

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

                return STATUS_OK;
            }
        } /* namespace wgl */
    } /* namespace r3d */
} /* namespace lsp */

