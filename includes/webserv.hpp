#pragma once

//Internal libraries
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <filesystem>

//Server
#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "CgiResponse.hpp"
#include "Monitor.hpp"
#include "Server.hpp"
