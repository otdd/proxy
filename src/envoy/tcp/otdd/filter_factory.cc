/* Copyright 2018 Istio Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "envoy/registry/registry.h"
#include "envoy/server/filter_config.h"
#include "src/envoy/tcp/otdd/filter.h"
#include "src/envoy/tcp/otdd/otdd_config.pb.h"

using ::otdd::config::OtddConfig;

namespace Envoy {
namespace Server {
namespace Configuration {

class OtddFilterFactory : public NamedNetworkFilterConfigFactory ,
			public Logger::Loggable<Logger::Id::filter>{
 public:
  Network::FilterFactoryCb createFilterFactory(
      const Json::Object& config_json, FactoryContext& context ) override {
    OtddConfig config_pb;
    config_pb.set_is_inbound(config_json.getBoolean("is_inbound"));
    config_pb.set_report_cluster(config_json.getString("report_cluster"));
    if (config_pb.report_cluster()==""){
      throw EnvoyException("Failed to parse JSON config, report_cluster is empty!");
    }
    return createFilterFactory(context,config_pb);
  }

  Network::FilterFactoryCb createFilterFactoryFromProto(
      const Protobuf::Message& config, FactoryContext& context ) override {
    return createFilterFactory(context,dynamic_cast<const OtddConfig&>(config));
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new OtddConfig};
  }

  std::string name() override { return "otdd"; }

 private:
  Network::FilterFactoryCb createFilterFactory(FactoryContext& context, const OtddConfig config_pb) {
    return [config_pb,&context](Network::FilterManager& filter_manager) -> void {
      std::shared_ptr<Tcp::Otdd::Filter> instance =
          std::make_shared<Tcp::Otdd::Filter>(config_pb,context);
      filter_manager.addReadFilter(Network::ReadFilterSharedPtr(instance));
      filter_manager.addWriteFilter(Network::WriteFilterSharedPtr(instance));
    };
  }
};

static Registry::RegisterFactory<OtddFilterFactory, NamedNetworkFilterConfigFactory>
    register_;

}  // namespace Configuration
}  // namespace Server
}  // namespace Envoy
