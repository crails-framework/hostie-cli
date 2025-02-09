#include "wizard.hpp"
#include <crails/cli/prompt.hpp>

using namespace std;

string telegraf_conf()
{
  ostringstream stream;
  const string host = Crails::prompt("URL of your InfluxDB instance:");
  const string token = Crails::prompt("InfluxDB token:");
  const string organization = Crails::prompt("InfluxDB Organziation:");
  const string bucket = Crails::prompt("InfluxDB Bucket:");
  const string interval("10s");

  cout
    << "Generating Telegraf settings." << endl
    << "Using InfluxDB_v2 at " << quoted(host) << " with token " << quoted(token) << endl;
  stream
    << "[agent]\n"
    << "  interval = " << quoted(interval) << '\n'
    << "  round_interval = true\n"
    << "  metric_batch_size = 1000\n"
    << "  metric_buffer_limit = 10000\n"
    << "  collection_jitter = \"0s\"\n"
    << "  flush_interval = " << quoted(interval) << '\n'
    << "  flush_jitter = \"0s\"\n"
    << "  precision = \"\"\n"
    << "  debug = false\n"
    << "  quiet = false\n"
    << "  hostname = \"\"\n"
    << "  omit_hostname = false\n"
    << '\n'
    << "[[inputs.cpu]]\n"
    << "  percpu = true\n"
    << "  totalcpu = true\n"
    << " fielddrop = [\"time_*\"]\n"
    << '\n'
    << "[[inputs.disk]]\n"
    << "  mount_points = [\"/\"]\n"
    << "  ignore_fs = [\"tmpfs\", \"devtmpfs\"]\n"
    << '\n'
    << "[[inputs.diskio]]\n"
    << "[[inputs.kernel]]\n"
    << "[[inputs.mem]]\n"
    << "[[inputs.processes]]\n"
    << "[[inputs.swap]]\n"
    << "[[inputs.system]]\n"
    << "[[inputs.net]]\n"
    << "# interfaces = [\"eth0\"]\n"
    << "[[inputs.netstat]]\n"
    << "[[inputs.interrupts]]\n"
    << "[[inputs.linux_sysctl_fs]]\n";
  stream
    << "[[outputs.influxdb_v2]]\n"
    << "  urls = [" << quoted(host) << "]\n"
    << "  token = " << quoted(token) << '\n'
    << "  organization = " << quoted(organization) << '\n'
    << "  bucket = " << quoted(bucket) << '\n';
  return stream.str();
}
