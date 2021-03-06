#include "CgiHandler.hpp"

CgiHandler::CgiHandler(void)
{
  return ;
}

CgiHandler::~CgiHandler(void)
{
  return ;
}

void  CgiHandler::_deleteEnv(std::vector<char *> & env)
{
  std::vector<char *>::iterator it;

  for (it = env.begin(); it != env.end(); ++it)
  {
    delete [] *it;
  }
  delete &env;
  return ;
}

void  CgiHandler::_addEnvVar(std::vector<char *> & env, std::string const & var)
{
  char *      aux;
  std::size_t len;

  len = var.length();
  aux = new char [len + 1];
  var.copy(aux, len);
  aux[len] = 0;
  env.push_back(aux);
  return ;
}

std::vector<char *> *
CgiHandler::getEnv(std::map<std::string, std::string> const & reqHeader,
                    std::map<std::string, std::string> const & urlData,
                    std::string const & ip)
{
  std::vector<char *> *                               env;
  std::map<std::string, std::string>::const_iterator  headerIt;
  std::map<std::string, std::string>::const_iterator  urlDataIt;
  std::string                                         content;

  env = new std::vector<char *>();
  content = "Auth_Type";
  this->_addEnvVar(*env, content);
  content = "Content_Length";
  headerIt = reqHeader.find("Content-Length");
  if (headerIt != reqHeader.end())
    content += '=' + headerIt->second;
  this->_addEnvVar(*env, content);
  content = "Content_Type";
  headerIt = reqHeader.find("Content-Type");
  if (headerIt != reqHeader.end())
   content += '=' + headerIt->second;
  this->_addEnvVar(*env, content);
  content = "Gateway_Interface=CGI/1.1";
  this->_addEnvVar(*env, content);
  content = "Path_Info";
  urlDataIt = urlData.find("Path_Info");
  if (urlDataIt != urlData.end())
    content += '=' + urlDataIt->second;
  this->_addEnvVar(*env, content);
  content = "Path_Translated";
  if (urlDataIt != urlData.end())
    content += '=' + /* location.root + */ urlDataIt->second;
  this->_addEnvVar(*env, content);
  content = "Query_String";
  urlDataIt = urlData.find("Query_String");
  if (urlDataIt != urlData.end())
    content += '=' + urlDataIt->second;
  this->_addEnvVar(*env, content);
  content = "Remote_Addr";
  if (!ip.empty())
    content += '=' + ip;
  this->_addEnvVar(*env, content);
  content = "Remote_Host";
  this->_addEnvVar(*env, content);
  //REMOTE_IDENT unset. Not mandatory
  //REMOTE_USER unset. Not mandatory
  content = "Request_Method=" + reqHeader.find("Method")->second;
  this->_addEnvVar(*env, content);
  /*
  **  Check if need to add cgi_bin folder path at the front.
  **
  **  This key must exist, as the presence of a valid CGI script filename
  **  in the request uri activates CGI processing operations.
  */
  content = "Script_Name=" + urlData.find("FileName")->second;
  this->_addEnvVar(*env, content);
  /*
  **  Add Server_Name and Port pairs to req's header map
  **  after parsing request. Then add their values to env's
  **  Server_Name and Server_Port respectively.
  */
  content = "Server_Name=" + reqHeader.find("Host")->second;
  this->_addEnvVar(*env, content);
  content = "Server_Port=" + reqHeader.find("Host")->second;
  this->_addEnvVar(*env, content);
  content = "Server_Protocol=HTTP/1.1";
  this->_addEnvVar(*env, content);
  content = "Server_Software=42WebServer/1.0";
  this->_addEnvVar(*env, content);
  //Protocol headers unset. Not mandatory
  return (env);
}

/*
**  Receive data from a cgi pipe's read end.
**
**  Need to check Content-length header from cgi response to know when
**  all the data from cgi program has been read.
*/

bool  CgiHandler::receiveData(int rPipe, ConnectionData & connData)
{
  std::size_t	endContent;
  int         len;

  endContent = connData.rspBuffSize;
	if (connData.rspBuffOffset)
	{ //This portion of code is equal to the one in Response::readFileNext
		//Move the content after offset to the front
		connData.rspBuff.replace(connData.rspBuff.begin(),
			connData.rspBuff.begin() + connData.rspBuffOffset,
			&connData.rspBuff[connData.rspBuffOffset]);
		//Fill the content that is duplicated at the back with NULL
		connData.rspBuff.replace(endContent - connData.rspBuffOffset,
			connData.rspBuffOffset, 0);
		//Update endContent
		endContent = endContent - connData.rspBuffOffset;
		//Reset offset
		connData.rspBuffOffset = 0;
	}
  len = read(rPipe, &connData.rspBuff[endContent],
													ConnectionData::rspBuffCapacity - endContent);
  if (len == 0)
  { //EOF
    std::cout << "Pipe write end closed. No more data to read." << std::endl;
    if (!connData.rspSize)
      connData.rspSize = connData.totalBytesRead;
    return (true);
  }
  // An error occurred. EAGAIN is not possible, because POLLIN was emitted.
  if (len < 0)
    return (false);
  if (!connData.totalBytesRead) //First call to receiveData
  {
    /*
    **  Provisional. rspSize must be obtained by parsing the Content-length
    **  header sent by cgi program
    */
    /*
    **  Here there should be a call to another method that parses
    **  the cgi response headers
    */
    connData.rspSize = 0; //Indicates unknown size
  }
  connData.rspBuffSize = endContent + len;
	connData.totalBytesRead += len;
  return (true);
}

// execve fails with std::vector<char *> & env

void  CgiHandler::_execProgram(CgiData const & cgiData,
                                ConnectionData & connData,
                                std::vector<char *> env)
{
  std::string programPath;
  char **     argv;

  programPath = connData.getLocation().cgi_dir + '/';
  programPath.append(connData.urlData.find("FileName")->second);
  argv = new char *[1 + 1];
  argv[1] = 0;
  argv[0] = const_cast<char *>(programPath.c_str());
  dup2(cgiData.inPipe[0], STDIN_FILENO);
  close(cgiData.inPipe[0]);
  dup2(cgiData.outPipe[1], STDOUT_FILENO);
  close(cgiData.outPipe[1]);
  execve(programPath.c_str(), argv, &env[0]);
  delete [] argv;
}

bool  CgiHandler::initPipes(CgiData & cgiData, ConnectionData & connData,
                            std::vector<char *> & env)
{
  pid_t child;

  if (pipe(cgiData.inPipe) == -1
      || pipe(cgiData.outPipe) == -1)
  {
    this->_deleteEnv(env);
    return (false);
  }
  child = fork();
  if (child < 0)
  {
    this->_deleteEnv(env);
    return (false);
  }
  if (!child)
  { //Child process
    close(cgiData.inPipe[1]);
    close(cgiData.outPipe[0]);
    this->_execProgram(cgiData, connData, env);
    std::cout << "exec failed" << std::endl;
    exit(EXIT_FAILURE);
  }
  else
  { //Parent process
    close(cgiData.inPipe[0]);
    close(cgiData.outPipe[1]);
    this->_deleteEnv(env);
    return (true);
  }
}
