#pragma once
#include "SinglePortModule.h"

class NodeCommandModule : public SinglePortModule {
public:
    NodeCommandModule();
protected:
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;
private:
    void sendTextReply(const meshtastic_MeshPacket &original, const char *text);
};

extern NodeCommandModule *nodeCommandModule;
