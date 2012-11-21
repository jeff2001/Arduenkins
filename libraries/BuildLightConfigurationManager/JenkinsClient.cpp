#include "JenkinsClient.h"
#include <string.h>
#include <stdlib.h>

#define JENKINS_POST_JOB_URL "/api/json?tree=color"
#define LINK_JOBS 2
#define DEBUG_JENKINS_CLIENT //Uncomment to print debug statements over serial

uint16_t JenkinsClient::getStatusForJob(JenkinsJob * job, EthernetClient * client) {
  uint16_t disposition = 0;

#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Making request to  IP:"));
  printIp(job->m_ip);
  Serial.println();
#endif

  // if you get a connection, report back via serial:
  if (client->connect(job->m_ip, job->m_port)) {
#ifdef DEBUG_JENKINS_CLIENT  
    Serial.print(F("connected\n"));
    // Make a HTTP request:
    Serial.print(F("GET "));
    Serial.print(job->m_jobLocations[0]);
    Serial.println(JENKINS_POST_JOB_URL);
#endif

    client->print("GET ");
    client->print(job->m_jobLocations[0]);
    client->println(JENKINS_POST_JOB_URL);
  } 
  else {
    // if you didn't get a connection to the server:
#ifdef DEBUG_JENKINS_CLIENT  
    Serial.print(F("connection failed\n"));
#endif
    client->stop();
    return JOB_INVALID_STATUS;
  }
  
  while (!client->available()) {
    //wait
  }
  
  char status[31] = {'\0'};
  int pos = 0;
  
  //assuming that the project name won't have a } in it.
  int bytesRead = client->readBytesUntil('}',status,30);
  status[bytesRead] = NULL;
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.println(status);
#endif
  client->flush();
  
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.println();
  Serial.println(F("disconnecting."));
#endif
  client->stop();
  
  char prefix[] = "{\"color\":\"";
  
  if(!strncmp(status, prefix, strlen(prefix))==0){
    return JOB_INVALID_STATUS;
  }
  
  disposition |= (strstr(status, "disabled") != NULL) ? JOB_DISABLED : 0;
  disposition |= (strstr(status, "blue") != NULL) ? JOB_SUCCEEDED : 0;
  disposition |= (strstr(status, "red") != NULL) ? JOB_FAILED : 0;
  disposition |= (strstr(status, "yellow") != NULL) ? JOB_UNSTABLE : 0;
  disposition |= (strstr(status, "grey") != NULL) ? JOB_CANCELED : 0;
  disposition |= (strstr(status, "anime") != NULL) ? JOB_IN_PROGRESS : 0;
  
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Found status: "));
  Serial.println(status);
  Serial.print(F("Mapped to disposition: "));
  Serial.println(disposition, BIN);
#endif

  return disposition;
}

void JenkinsClient::printIp(uint8_t ip[]) {
  for(int i = 0 ; i < 4 ; i++){
    Serial.print(ip[i]);
    if(i<3) { Serial.print(F(".")); }
  }
}

