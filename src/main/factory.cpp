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

#include <lsp-plug.in/common/types.h>
#include <lsp-plug.in/r3d/wgl/backend.h>
#include <lsp-plug.in/r3d/wgl/factory.h>
#include <stdlib.h>

namespace lsp
{
    namespace r3d
    {
        namespace wgl
        {
            const r3d::backend_metadata_t factory_t::sMetadata[] =
            {
                { "wgl_2x", "openGL 2.0+ (Windows)", "wgl_opengl_v2" }
            };

            const r3d::backend_metadata_t *factory_t::metadata(r3d::factory_t *handle, size_t id)
            {
                size_t count = sizeof(sMetadata) / sizeof(r3d::backend_metadata_t);
                return (id < count) ? &sMetadata[id] : NULL;
            }

            r3d::backend_t *factory_t::create(r3d::factory_t *handle, size_t id)
            {
                if (id == 0)
                {
                    wgl::backend_t *res = static_cast<wgl::backend_t *>(::malloc(sizeof(wgl::backend_t)));
                    if (res != NULL)
                        res->construct();
                    return res;
                }
                return NULL;
            }

            factory_t::factory_t()
            {
                #define R3D_WGL_FACTORY_EXP(func)   r3d::factory_t::func = factory_t::func;
                R3D_WGL_FACTORY_EXP(create);
                R3D_WGL_FACTORY_EXP(metadata);
                #undef R3D_WGL_FACTORY_EXP
            }

            factory_t::~factory_t()
            {
            }

        } /* namespace wgl */
    } /* namespace r3d */
} /* namespace lsp */


