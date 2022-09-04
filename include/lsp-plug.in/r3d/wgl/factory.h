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

#ifndef LSP_PLUG_IN_R3D_WGL_FACTORY_H_
#define LSP_PLUG_IN_R3D_WGL_FACTORY_H_

#include <lsp-plug.in/r3d/wgl/version.h>

#include <lsp-plug.in/r3d/iface/factory.h>
#include <lsp-plug.in/r3d/wgl/version.h>

namespace lsp
{
    namespace r3d
    {
        namespace wgl
        {
            // GLX backend factory
            typedef struct factory_t: public r3d::factory_t
            {
                static const r3d::backend_metadata_t    sMetadata[];

                static const r3d::backend_metadata_t   *metadata(r3d::factory_t *_this, size_t id);
                static r3d::backend_t                  *create(r3d::factory_t *_this, size_t id);

                explicit factory_t();
                ~factory_t();

            } factory_t;

        } /* namespace wgl */
    } /* namespace r3d */
} /* namespace lsp */

#endif /* LSP_PLUG_IN_R3D_WGL_FACTORY_H_ */
