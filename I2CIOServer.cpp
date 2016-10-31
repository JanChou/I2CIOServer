/**
 * (c)2016 CDP Technologies AS
 */

#include "I2CIOServer.h"

#include "I2CAdapter.h"
#include "I2CDevice.h"
#include "I2CDeviceFactory.h"

#include <IO/ServerIO/ChannelManager.h>
#include <OSAPI/Process/OSAPIThread.h>

using namespace CDP::StudioAPI;
using namespace std;

// The Thread provides std::thread like interface for OSAPIThread
class Thread : public OSAPIThread
{
public:
  Thread(const function<void()>& function)
    : m_function(function)
  {
  }

protected:
  void Main() override
  {
    m_function();
  }

private:
  function<void()> m_function;
};

class I2CIOServer::Private
{
public:
  Private(I2CIOServer* qq)
    : channelManager(new ServerIO::ChannelManager(qq))
    , worker(bind(&Private::Process, this))
  {
  }

  void Initialize()
  {
    i2cAdapter.Open(adapter);
    for (auto device : devices)
      device->Initialize(i2cAdapter);
  }

  void Process()
  {
    while (!worker.Stopped())
    {
      semaphore.Wait();
      semaphore.Reset();

      channelManager->SynchronizeValuesIn();
      for (auto device : devices)
        device->Process(i2cAdapter);
      channelManager->SynchronizeValuesOut();
    }
  }

  void Dispose()
  {
    i2cAdapter.Close();
  }

  unique_ptr<ServerIO::ChannelManager> channelManager;
  vector<I2CDevice*> devices;

  CDPProperty<string> adapter;

  I2CAdapter i2cAdapter;
  Thread worker;

  OSAPIEvent semaphore;
};

I2CIOServer::I2CIOServer()
  : d(new Private(this))
{
}

I2CIOServer::~I2CIOServer()
{
  for (auto device : d->devices)
    delete device;
}

void I2CIOServer::Create(const char* fullName)
{
  IOServer::Create(fullName);

  d->adapter.Create("Adapter", this, CDPPropertyBase::e_Attribute);
}

void I2CIOServer::ProcessNull()
{
  IOServer::ProcessNull();
  d->semaphore.Set();
}

void I2CIOServer::ProcessOffline()
{
  IOServer::ProcessOffline();
  d->semaphore.Set();
}

void I2CIOServer::ProcessOnline()
{
  IOServer::ProcessOnline();
  d->semaphore.Set();
}

bool I2CIOServer::IsCommProblem()
{
  return false;
}

void I2CIOServer::Activate()
{
  d->Initialize();
  d->worker.Start(CDPTHREAD_PRIORITY_HIGH, ShortName());
  IOServer::Activate();
}

void I2CIOServer::Suspend()
{
  IOServer::Suspend();
  d->worker.Stop();
  d->semaphore.Set();
}

void I2CIOServer::FillNodeChildren(NodeStream& serializer) const
{
  IOServer::FillNodeChildren(serializer);
  serializer.StdContainerStreamer(d->devices);
}

string I2CIOServer::GetNodeTypeName() const
{
  return "I2CIOServer";
}

bool I2CIOServer::HandleXMLElement(XMLElementEx* pEx)
{
  if (pEx->GetName() == "Device")
  {
    auto device = I2CDeviceFactory::CreateDevice(pEx->GetAttributeValue("Model"));
    device->Configure(pEx, this, d->channelManager.get());
    d->devices.push_back(device);
    return true;
  }
  return IOServer::HandleXMLElement(pEx);
}
