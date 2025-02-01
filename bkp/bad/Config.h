#ifndef CONFIG_H
#define CONFIG_H

#include <FS.h>
#include <SPIFFS.h>

class Config {
public:
    Config();
    void begin();
    void loadConfig();
    void saveConfig();
    int getNumeroMessage() const;
    void setNumeroMessage(int numeroMessage);

private:
    int numeroMessage;
};

#endif // CONFIG_H
