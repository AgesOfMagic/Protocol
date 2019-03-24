#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PACKET_SIZE 512

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

//Server ids
#define HAND_SHAKE_SERVER_ID 0x00
#define UPDATE_POSITION_ID 0x01

//enums
typedef enum PacketTypes {
  UNKNOWN,
  HAND_SHAKE_CLIENT_TYPE,
  HAND_SHAKE_SERVER_TYPE,
  MOVEMENT_PACKET_TYPE,
  UPDATE_POSITION_TYPE,
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


//Server structs
typedef struct HandShakeServer {
  Status status;
  unsigned char serverVersion[16];
  unsigned char characterData[492];
} HandShakeServer;

typedef struct UpdatePosition {
  int x;
  int y;
  unsigned int id;
} UpdatePosition;


//utils
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
    else if (UPDATE_POSITION_ID){
        return UPDATE_POSITION_TYPE;
    }
  }else {
    if(id == HAND_SHAKE_CLIENT_ID){
      return HAND_SHAKE_CLIENT_TYPE;
    }
    else if(id == MOVEMENT_PACKET_ID){
      return MOVEMENT_PACKET_TYPE;
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

  for (int i = 0; i < 492; i++)
  {
    buff[i + 5 + 16] = serverPacket.characterData[i];
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
  strcpy(server.characterData, cutStr(buff,5 + 16, 492));
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
