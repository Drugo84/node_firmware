#include "NodeCommandModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PositionModule.h"
#include "Router.h"
#include "configuration.h"
#include "main.h"
#include "modules/Telemetry/DeviceTelemetry.h"

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

    char cmdPing[64], cmdPos[64], cmdMetrics[64];
    snprintf(cmdPing,    sizeof(cmdPing),    "%s ping",      myName);
    snprintf(cmdPos,     sizeof(cmdPos),     "%s position", myName);
    snprintf(cmdMetrics, sizeof(cmdMetrics), "%s telemetry",  myName);

    LOG_INFO("NodeCommandModule: ricevuto '%s' da 0x%x\n", incoming, mp.from);

    if (strncasecmp(incoming, cmdPing, strlen(cmdPing)) == 0) {
        sendTextReply(mp, "pong");
        return ProcessMessage::STOP;

    } else if (strncasecmp(incoming, cmdPos, strlen(cmdPos)) == 0) {
        if (positionModule)
            positionModule->sendOurPosition();
        sendTextReply(mp, "Aggiornamento posizione inviato");
        return ProcessMessage::STOP;

    } else if (strncasecmp(incoming, cmdMetrics, strlen(cmdMetrics)) == 0) {
        if (deviceTelemetryModule)
            deviceTelemetryModule->sendTelemetryPublic();
        sendTextReply(mp, "Aggiornamento metriche inviato");
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
    reply->to      = original.from;
    reply->channel = original.channel;

    service->sendToMesh(reply, RX_SRC_LOCAL);
}
