#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PACKET_SIZE 512
#define HEADER_SIZE 128


/*
  to add Packet:
  1) create new struct
  2) add to PacketTypes
  3) add to buff and from buff
  4) add to identify
*/

//Client ids
#define HAND_SHAKE_CLIENT_ID 0x00
#define MOVEMENT_PACKET_ID 0x01
#define BUILD_PACKET_ID 0x02

//Server ids
#define HAND_SHAKE_SERVER_ID 0x00
#define UPDATE_POSITION_ID 0x01
#define UPDATE_FILE_HEADER_ID 0x02
#define UPDATE_WORLD_ID 0x03

//enums
typedef enum PacketTypes {
  UNKNOWN,
  HAND_SHAKE_CLIENT_TYPE,
  HAND_SHAKE_SERVER_TYPE,
  MOVEMENT_PACKET_TYPE,
  UPDATE_POSITION_TYPE,
  UPDATE_FILE_HEADER_TYPE,
  BUILD_PACKET_TYPE,
  UPDATE_WORLD_TYPE,
} PacketTypes;

typedef enum Status {
  UNRECOGNIZED, // => 0
  OK, // => 1
  UNMATCHED_VERSION,
  UNKNOWON_CHARACTER
} Status;

typedef enum Direction {
  NORTH,
  SOUTH,
  WEST,
  EAST
} Direction;

//Client structs
typedef struct HandShakeClient {
  unsigned char displayName[128];
  unsigned char clientVersion[16];
  unsigned char characterSecret[32];
} HandShakeClient;

typedef struct MovementPacket {
  Direction direction;
  unsigned char characterSecret[32];
} MovementPacket;

typedef struct BuildPacket {
  unsigned short buildId;
  long x;
  long y;
} BuildPacket;



//Server structs
typedef struct HandShakeServer {
  Status status;
  unsigned char serverVersion[16];
  unsigned char serverName[128];
} HandShakeServer;

typedef struct UpdatePosition {
  long x;
  long y;
  unsigned long id;
} UpdatePosition;

typedef struct UpdateWorld {
  unsigned long blockID;
  long x;
  long y;
} UpdateWorld;

typedef struct UpdateFileHeader
{
  unsigned long length;
  unsigned char name[72];
} UpdateFileHeader;

//utils
unsigned short calculateBigPacket(unsigned long long length){
  return (unsigned short)(length / PACKET_SIZE);
}

unsigned char* cutStr(unsigned  char str[], int offset, int length){
  static unsigned char r[PACKET_SIZE] = {0};
  for (int i = 0; i < length; i++) {
    r[i] = str[i + offset];
  }
  unsigned char *toReturn = r;
  return toReturn;
}

PacketTypes identify(unsigned char id, int is_from_sever){
  if (is_from_sever) {
    if(id == HAND_SHAKE_SERVER_ID){
      return HAND_SHAKE_SERVER_TYPE;
    }
    else if (id == UPDATE_POSITION_ID){
      return UPDATE_POSITION_TYPE;
    }
    else if (id == UPDATE_FILE_HEADER_ID){
      return UPDATE_FILE_HEADER_TYPE;
    }else if (id == UPDATE_WORLD_ID){
      return  UPDATE_WORLD_TYPE;
    }
  }else {
    if(id == HAND_SHAKE_CLIENT_ID){
      return HAND_SHAKE_CLIENT_TYPE;
    }
    else if(id == MOVEMENT_PACKET_ID){
      return MOVEMENT_PACKET_TYPE;
    } else if(id == BUILD_PACKET_ID){
      return  BUILD_PACKET_TYPE;
    }
  }
  return UNKNOWN;
}

//Client Packet Funcs def
unsigned char* handShakeClientToBuffer(HandShakeClient clientPacket)
{
  static unsigned char buff[PACKET_SIZE] = {0};
  buff[0] = HAND_SHAKE_CLIENT_ID;
  for (int i = 0; i < 128; i++) {
    buff[i + 1] = clientPacket.displayName[i];
  }

  for (int i = 0; i < 16; i++)
  {
    buff[i + 128 + 1] = clientPacket.clientVersion[i];
  }

  for (int i = 0; i < 32; i++)
  {
    buff[i + 128 + 16 + 1] = clientPacket.characterSecret[i];
  }
  // buff[128 + 16 + 32] = '\0';
  unsigned char* t = buff;

  return t;
}

HandShakeClient bufferToHandShakeClient(unsigned char buff[])
{

  HandShakeClient client = {0, 0, 0};
  strcpy(client.displayName, cutStr(buff,1,128));
  strcpy(client.clientVersion, cutStr(buff,129,16));
  strcpy(client.characterSecret, cutStr(buff,129 + 16, 32));
  return client;
}

//Server Packet funcs def
unsigned char* handShakeServerToBuffer(HandShakeServer serverPacket)
{
  static unsigned char buff[PACKET_SIZE] = {0};
  buff[0] = HAND_SHAKE_SERVER_ID;
  buff[1] = (serverPacket.status >> 24) & 0xFF;
  buff[2] = (serverPacket.status >> 16) & 0xFF;
  buff[3] = (serverPacket.status >> 8) & 0xFF;
  buff[4] = serverPacket.status & 0xFF;

  for (int i = 0; i < 16; i++)
  {
    buff[i + 5] = serverPacket.serverVersion[i];
  }

  for (int i = 0; i < 128; i++)
  {
    buff[i + 5 + 16] = serverPacket.serverName[i];
  }
  // buff[128 + 16 + 32] = '\0';
  unsigned char* t = buff;

  return t;
}

HandShakeServer bufferToHandShakeServer(unsigned char buff[])
{
  HandShakeServer server = {0, 0, 0};
  Status s = buff[4] | (Status)buff[3] << 8
    | (Status)buff[2] << 16 | (Status)buff[1] << 24;
  server.status = s;
  strcpy(server.serverVersion, cutStr(buff,5,16));
  strcpy(server.serverName, cutStr(buff,5 + 16, 128));
  return server;
}

// MovementPacket Funcs
unsigned char* movementToBuffer(MovementPacket movementPacket)
{
  static unsigned char buffer[PACKET_SIZE] = {0};
  buffer[0] = MOVEMENT_PACKET_ID;
  buffer[1] = (movementPacket.direction >> 24) & 0xFF;
  buffer[2] = (movementPacket.direction >> 16) & 0xFF;
  buffer[3] = (movementPacket.direction >> 8) & 0xFF;
  buffer[4] = movementPacket.direction & 0xFF;

  for (int i = 0; i < 32; i++)
  {
    buffer[i + 5] = movementPacket.characterSecret[i];
  }
  unsigned char* t = buffer;
  return t;
}

MovementPacket bufferToMovement(unsigned char buff[])
{
  MovementPacket packet = {0, 0};
  Status d = buff[4] | (Status)buff[3] << 8
    | (Status)buff[2] << 16 | (Status)buff[1] << 24;
  packet.direction = d;
  strcpy(packet.characterSecret, cutStr(buff,5,32));
  return packet;
}

// UpdateFiles Funcs
unsigned char* updateFileHeaderToBuffer(UpdateFileHeader updateFileHeader)
{
  static unsigned char buffer[HEADER_SIZE] = {0};
  buffer[0] = UPDATE_FILE_HEADER_ID;
  buffer[1] = (updateFileHeader.length >> 24) & 0xFF;
  buffer[2] = (updateFileHeader.length >> 16) & 0xFF;
  buffer[3] = (updateFileHeader.length >> 8) & 0xFF;
  buffer[4] = updateFileHeader.length  & 0xFF;
  for (int i = 0; i < 72; i++)
  {
    buffer[i + 5] = updateFileHeader.name[i];
  }

  unsigned char* t = buffer;
  return t;
}

UpdateFileHeader bufferToUpdateFileHeader(unsigned char buff[])
{
  UpdateFileHeader packet = {0, 0, 0};
  unsigned long l = (unsigned long)buff[4] | (unsigned long)buff[3] << 8
    | (unsigned long)buff[2] << 16 | (unsigned long)buff[1] << 24;
  packet.length = l;
  strcpy(packet.name, cutStr(buff,5,72));
  return packet;
}

// UpdatePos Funcs
unsigned char* updatePositionToBuffer(UpdatePosition updatePosition)
{
  static unsigned char buffer[PACKET_SIZE] = {0};
  buffer[0] = UPDATE_POSITION_ID;
  buffer[1] = (updatePosition.x >> 24) & 0xFF;
  buffer[2] = (updatePosition.x >> 16) & 0xFF;
  buffer[3] = (updatePosition.x >> 8) & 0xFF;
  buffer[4] = updatePosition.x & 0xFF;

  buffer[5] = (updatePosition.y >> 24) & 0xFF;
  buffer[6] = (updatePosition.y >> 16) & 0xFF;
  buffer[7] = (updatePosition.y >> 8) & 0xFF;
  buffer[8] = updatePosition.y & 0xFF;

  buffer[9] = (updatePosition.id >> 24) & 0xFF;
  buffer[10] = (updatePosition.id >> 16) & 0xFF;
  buffer[11] = (updatePosition.id >> 8) & 0xFF;
  buffer[12] = updatePosition.id & 0xFF;
  unsigned char* t = buffer;
  return t;
}

UpdatePosition bufferToUpdatePosition(unsigned char buff[])
{
  UpdatePosition packet = {0, 0, 0};
  int d = buff[4] | (int)buff[3] << 8
    | (int)buff[2] << 16 | (int)buff[1] << 24;
  packet.x = d;
  int b = buff[8] | (int)buff[7] << 8
    | (int)buff[6] << 16 | (int)buff[5] << 24;
  packet.y = b;
  int t = buff[12] | (int)buff[11] << 8
    | (int)buff[10] << 16 | (int)buff[9] << 24;
  packet.id = t;
  return packet;
}

//build
unsigned char* buildPacketToBuffer(BuildPacket buildPacket)
{
  static unsigned char buffer[PACKET_SIZE] = {0};
  buffer[0] = UPDATE_POSITION_ID;
  buffer[1] = (buildPacket.x >> 24) & 0xFF;
  buffer[2] = (buildPacket.x >> 16) & 0xFF;
  buffer[3] = (buildPacket.x >> 8) & 0xFF;
  buffer[4] = buildPacket.x & 0xFF;

  buffer[5] = (buildPacket.y >> 24) & 0xFF;
  buffer[6] = (buildPacket.y >> 16) & 0xFF;
  buffer[7] = (buildPacket.y >> 8) & 0xFF;
  buffer[8] = buildPacket.y & 0xFF;

  buffer[9] = (buildPacket.buildId >> 8) & 0xFF;
  buffer[10] = buildPacket.buildId & 0xFF;
  unsigned char* t = buffer;
  return t;
}

BuildPacket bufferToBuildPacket(unsigned char buff[])
{
  BuildPacket packet = {0, 0, 0};
  int d = buff[4] | (int)buff[3] << 8
    | (int)buff[2] << 16 | (int)buff[1] << 24;
  packet.x = d;
  int b = buff[8] | (int)buff[7] << 8
    | (int)buff[6] << 16 | (int)buff[5] << 24;
  packet.y = b;
  unsigned short t = buff[10] | (unsigned short)buff[9] << 8;
  packet.buildId = t;
  return packet;
}

//update World
unsigned char* updateWorldToBuffer(UpdateWorld updateWorld)
{
  static unsigned char buffer[PACKET_SIZE] = {0};
  buffer[0] = UPDATE_POSITION_ID;
  buffer[1] = (updateWorld.x >> 24) & 0xFF;
  buffer[2] = (updateWorld.x >> 16) & 0xFF;
  buffer[3] = (updateWorld.x >> 8) & 0xFF;
  buffer[4] = updateWorld.x & 0xFF;

  buffer[5] = (updateWorld.y >> 24) & 0xFF;
  buffer[6] = (updateWorld.y >> 16) & 0xFF;
  buffer[7] = (updateWorld.y >> 8) & 0xFF;
  buffer[8] = updateWorld.y & 0xFF;

  buffer[9] = (updateWorld.blockID >> 24) & 0xFF;
  buffer[10] = (updateWorld.blockID >> 16) & 0xFF;
  buffer[11] = (updateWorld.blockID >> 8) & 0xFF;
  buffer[12] = updateWorld.blockID & 0xFF;
  unsigned char* t = buffer;
  return t;
}

UpdateWorld bufferToUpdateWorld(unsigned char buff[])
{
  UpdateWorld packet = {0, 0, 0};
  int d = buff[4] | (int)buff[3] << 8
    | (int)buff[2] << 16 | (int)buff[1] << 24;
  packet.x = d;
  int b = buff[8] | (int)buff[7] << 8
    | (int)buff[6] << 16 | (int)buff[5] << 24;
  packet.y = b;
  unsigned long t = buff[12] | (unsigned long)buff[11] << 8
    | (unsigned long)buff[10] << 16 | (unsigned long)buff[9] << 24;
  packet.blockID = t;
  return packet;
}