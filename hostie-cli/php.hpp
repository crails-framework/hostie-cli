#pragma once
#include <filesystem>

class InstanceEnvironment;

std::string php_version();
std::filesystem::path fpm_pool_path(const InstanceEnvironment&);
