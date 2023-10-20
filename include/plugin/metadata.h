// BSD 4-Clause License
//
// Copyright (c) 2023, UltiMaker
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// 3. All advertising materials mentioning features or use of this software must
//   display the following acknowledgement:
//     This product includes software developed by UltiMaker.
//
// 4. Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLUGIN_METADATA_H
#define PLUGIN_METADATA_H

#include "plugin/cmdline.h"

#include <agrpc/asio_grpc.hpp>
#include <spdlog/spdlog.h>

#include <string_view>

namespace plugin
{

struct Metadata
{
    std::string_view slot_version_range{ ">=0.1.0 <0.2.0" };
    std::string_view plugin_name{ cmdline::NAME };
    std::string_view plugin_version{ cmdline::VERSION };
};

std::string getUuid(grpc::ServerContext& server_context)
{
    auto c_uuid = server_context.client_metadata().find("cura-engine-uuid");
    if (c_uuid == server_context.client_metadata().end())
    {
        spdlog::error("cura-engine-uuid not found in client metadata");
        throw std::runtime_error("cura-engine-uuid not found in client metadata");
    }
    return { c_uuid->second.data(), c_uuid->second.size() };
}


} // namespace plugin

#endif // PLUGIN_METADATA_H
