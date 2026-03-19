#include "NodeCommandModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PositionModule.h"
#include "Router.h"
#include "configuration.h"
#include "main.h"
#include "modules/Telemetry/DeviceTelemetry.h"
#include "modules/Telemetry/EnvironmentTelemetry.h"

NodeCommandModule *nodeCommandModule;

NodeCommandModule::NodeCommandModule()
    : SinglePortModule("NodeCommandModule", meshtastic_PortNum_TEXT_MESSAGE_APP)
{
}

ProcessMessage NodeCommandModule::handleReceived(const meshtastic_MeshPacket &mp)
{
    if (mp.decoded.payload.size == 0)
        return ProcessMessage::CONTINUE;

    char incoming[256] = {0};
    size_t len = mp.decoded.payload.size < sizeof(incoming) - 1
                     ? mp.decoded.payload.size
                     : sizeof(incoming) - 1;
    memcpy(incoming, mp.decoded.payload.bytes, len);

    const char *myName = owner.long_name;
    if (!myName || myName[0] == '\0')
        myName = "node";

    char cmdPing[64], cmdPos[64], cmdMetrics[64], cmdWhere[64];
    snprintf(cmdPing,    sizeof(cmdPing),    "%s ping",      myName);
    snprintf(cmdPos,     sizeof(cmdPos),     "%s position",  myName);
    snprintf(cmdMetrics, sizeof(cmdMetrics), "%s telemetry", myName);
    snprintf(cmdWhere,   sizeof(cmdWhere),   "%s where",     myName);

    LOG_INFO("NodeCommandModule: ricevuto '%s' da 0x%x\n", incoming, mp.from);

    if (strncasecmp(incoming, cmdPing, strlen(cmdPing)) == 0) {
        sendTextReply(mp, "pong");
        return ProcessMessage::STOP;

    } else if (strncasecmp(incoming, cmdPos, strlen(cmdPos)) == 0) {
        if (positionModule)
            positionModule->sendOurPosition();
        sendTextReply(mp, "Sent");
        return ProcessMessage::STOP;

    } else if (strncasecmp(incoming, cmdMetrics, strlen(cmdMetrics)) == 0) {
        if (deviceTelemetryModule)
            deviceTelemetryModule->sendTelemetryPublic();
        if (environmentTelemetryModule)
            environmentTelemetryModule->sendEnvTelemetryPublic();
        sendTextReply(mp, "Sent");
        return ProcessMessage::STOP;

    } else if (strncasecmp(incoming, cmdWhere, strlen(cmdWhere)) == 0) {
        meshtastic_NodeInfoLite *me = nodeDB->getMeshNode(nodeDB->getNodeNum());
        if (me && me->has_position && me->position.latitude_i != 0) {
            char link[120];
            snprintf(link, sizeof(link), "https://maps.google.com/?q=%.5f,%.5f",
                me->position.latitude_i * 1e-7,
                me->position.longitude_i * 1e-7);
            sendTextReply(mp, link);
        } else {
            sendTextReply(mp, "No GPS fix");
        }
        return ProcessMessage::STOP;
    }

    return ProcessMessage::CONTINUE;
}

void NodeCommandModule::sendTextReply(const meshtastic_MeshPacket &original, const char *text)
{
    meshtastic_MeshPacket *reply = router->allocForSending();
    reply->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;

    size_t textLen = strlen(text);
    if (textLen > sizeof(reply->decoded.payload.bytes) - 1)
        textLen = sizeof(reply->decoded.payload.bytes) - 1;

    memcpy(reply->decoded.payload.bytes, text, textLen);
    reply->decoded.payload.size = textLen;
    reply->to = (original.to == NODENUM_BROADCAST) ? NODENUM_BROADCAST : original.from;
    reply->channel = original.channel;

    service->sendToMesh(reply, RX_SRC_LOCAL);
}
