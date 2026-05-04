#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct FVDA5050Node
{
  std::string NodeId;
  uint32_t SequenceId;
  double X, Y;
  std::optional<double> Theta;
};

struct FVDA5050Order
{
  std::string OrderId;
  uint32_t OrderUpdateId;
  std::vector<FVDA5050Node> Nodes;
};

class VDAWRAPPER_API FVDA5050Client
{
public:
  
  FVDA5050Client();

  ~FVDA5050Client();

  bool Connect(
    const std::string & BrokerAddress,
    const std::string & InterfaceName,
    const std::string & Version,
    const std::string & Manufacturer,
    const std::string & SerialNumber
  );

  void Disconnect();

  void SpinOnce();

  void ClientNodeAck(uint32_t SequenceId);

  void SetPublishState(bool bEnabled);

  std::function<void (const FVDA5050Node &)> OnNodeDispatch;
  std::function<void (const FVDA5050Order &)> OnOrderReceived;
  std::function<void (double & X, double & Y, double & Theta)> OnPositionRequest;

private:
  struct FImpl;
  std::unique_ptr<FImpl> Impl;
};