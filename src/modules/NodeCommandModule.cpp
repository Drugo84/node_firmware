#include "NodeCommandModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PowerStatus.h"
#include "Router.h"
#include "configuration.h"
#include "main.h"

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

    char cmdPing[64], cmdTelemetry[64], cmdPosition[64];
    snprintf(cmdPing,      sizeof(cmdPing),      "%s ping",      myName);
    snprintf(cmdTelemetry, sizeof(cmdTelemetry), "%s telemetry", myName);
    snprintf(cmdPosition,  sizeof(cmdPosition),  "%s position",  myName);

    LOG_INFO("NodeCommandModule: received '%s' from 0x%x\n", incoming, mp.from);

    if (strncasecmp(incoming, cmdPing, strlen(cmdPing)) == 0) {
        uint8_t hops = (mp.hop_start > mp.hop_limit) ? (mp.hop_start - mp.hop_limit) : 0;
        char reply[80];
        snprintf(reply, sizeof(reply), "[%s] Pong | SNR:%.1fdB RSSI:%ddBm Hops:%d",
            owner.short_name, mp.rx_snr, mp.rx_rssi, hops);
        sendTextReply(mp, reply);
        return ProcessMessage::STOP;

    } else if (strncasecmp(incoming, cmdTelemetry, strlen(cmdTelemetry)) == 0) {
        meshtastic_NodeInfoLite *me = nodeDB->getMeshNode(nodeDB->getNodeNum());
        char reply[120];
        if (me && me->has_device_metrics) {
            uint32_t upS = millis() / 1000;
            snprintf(reply, sizeof(reply), "[%s] Bat:%d%% %.2fV Up:%dh%dm",
                owner.short_name,
                me->device_metrics.battery_level,
                me->device_metrics.voltage,
                upS / 3600, (upS % 3600) / 60);
        } else {
            snprintf(reply, sizeof(reply), "[%s] No data", owner.short_name);
        }
        sendTextReply(mp, reply);
        return ProcessMessage::STOP;

    } else if (strncasecmp(incoming, cmdPosition, strlen(cmdPosition)) == 0) {
        meshtastic_NodeInfoLite *me = nodeDB->getMeshNode(nodeDB->getNodeNum());
        if (me && me->has_position && me->position.latitude_i != 0) {
            char reply[120];
            snprintf(reply, sizeof(reply), "https://maps.google.com/?q=%.5f,%.5f",
                me->position.latitude_i * 1e-7,
                me->position.longitude_i * 1e-7);
            sendTextReply(mp, reply);
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
