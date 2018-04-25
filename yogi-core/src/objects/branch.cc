#include "branch.h"
#include "../utils/system.h"
#include "../utils/time.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

namespace objects {

Branch::Branch(ContextPtr context, std::string name, std::string description,
               std::string net_name, std::string password, std::string path,
               std::string interface, int adv_port,
               std::chrono::milliseconds adv_interval)
    : context_(context),
      uuid_(boost::uuids::random_generator()()),
      name_(name),
      description_(description),
      net_name_(net_name),
      password_(password),
      path_(path),
      interface_(interface),
      adv_port_(adv_port),
      adv_interval_(adv_interval),
      start_time_(utils::GetCurrentUtcTime()) {}

std::string Branch::MakeInfo() const {
  boost::property_tree::ptree pt;
  pt.put("uuid", boost::uuids::to_string(uuid_));
  pt.put("name", name_);
  pt.put("description", description_);
  pt.put("net_name", net_name_);
  pt.put("path", path_);
  pt.put("hostname", utils::GetHostname());
  pt.put("pid", utils::GetPid());
  pt.put("interface", interface_);
  pt.put("advertising_port", adv_port_);
  pt.put("advertising_interval", (float)adv_interval_.count() / 1000.0f);
  pt.put("start_time", utils::TimeToJavaScriptString(start_time_));
  pt.put("active_connections", 0);  // TODO

  std::stringstream oss;
  boost::property_tree::json_parser::write_json(oss, pt);
  return oss.str();
}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&)>& fn) const {}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&, std::string)>& fn)
    const {}

}  // namespace objects
