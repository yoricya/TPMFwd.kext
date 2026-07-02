//
//  TPMUserClient.h
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#ifndef TPMUserClient_h
#define TPMUserClient_h

#include <IOKit/IOUserClient.h>

class TPMFwd;

class TPMUserClient : public IOUserClient {
    OSDeclareDefaultStructors(TPMUserClient);

private:
    TPMFwd *fDriver;
    task_t fTask;

public:
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual IOReturn externalMethod(uint32_t selector,
                                    IOExternalMethodArguments *args,
                                    IOExternalMethodDispatch *dispatch = 0,
                                    OSObject *target = 0,
                                    void *reference = 0) override;
    
    virtual IOReturn clientClose() override;
    virtual IOReturn clientDied() override;
    
    void setDriver(TPMFwd *driver);
};

#endif
