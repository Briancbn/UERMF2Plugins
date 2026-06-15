/*
 * Copyright (C) 2025-2026 ROS-Industrial Consortium Asia Pacific
 * Advanced Remanufacturing and Technology Centre
 * A*STAR Research Entities (Co. Registration No. 199702110H)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MQTTActor.h"
#include "MQTTClient.h"

#include <string>

static int PahoMessageArrived(
    void* Context,
    char* TopicName,
    int TopicLen,
    MQTTClient_message* Message
)
{
  return AMQTTActor::OnMessageArrived(
      Context,
      TopicName,
      TopicLen,
      (void*)Message
  );
}

// Sets default values
AMQTTActor::AMQTTActor()
{
  // Set this actor to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMQTTActor::BeginPlay() { Super::BeginPlay(); }

// Called every frame
void AMQTTActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

MQTTClient AMQTTActor::Client = nullptr;
bool AMQTTActor::bIsConnected = false;
FString AMQTTActor::LastError = "";
TArray<AMQTTActor::FMQTTMessage> AMQTTActor::MessageQueue;
FCriticalSection AMQTTActor::MessageQueueLock;

bool AMQTTActor::ConnectToMQTTBroker(
    const FString& BrokerAddress,
    const FString& ClientID,
    const FString& Username,
    const FString& Password,
    int32 KeepAliveInterval,
    bool bCleanSession
)
{
  // Disconnect if already connected
  if (Client != nullptr)
  {
    DisconnectFromMQTTBroker();
  }

  // Convert FString to const char*
  std::string BrokerAddr = TCHAR_TO_UTF8(*BrokerAddress);
  std::string ClientId = TCHAR_TO_UTF8(*ClientID);

  // Create MQTT client
  int rc = MQTTClient_create(
      &Client,
      BrokerAddr.c_str(),
      ClientId.c_str(),
      MQTTCLIENT_PERSISTENCE_NONE,
      nullptr
  );

  if (rc != MQTTCLIENT_SUCCESS)
  {
    LastError = FString::Printf(TEXT("Failed to create MQTT client: %d"), rc);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LastError);
    return false;
  }

  // Set callbacks
  MQTTClient mqttClient = (MQTTClient)Client;
  MQTTClient_setCallbacks(
      mqttClient,
      nullptr,
      OnConnectionLost,
      PahoMessageArrived,
      OnDeliveryComplete
  );

  // Set connection options
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = KeepAliveInterval;
  conn_opts.cleansession = bCleanSession ? 1 : 0;

  // Set username and password if provided
  if (!Username.IsEmpty())
  {
    std::string User = TCHAR_TO_UTF8(*Username);
    std::string Pass = TCHAR_TO_UTF8(*Password);
    conn_opts.username = User.c_str();
    conn_opts.password = Pass.c_str();
  }

  // Connect
  rc = MQTTClient_connect(mqttClient, &conn_opts);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    LastError = FString::Printf(TEXT("Failed to connect to broker: %d"), rc);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LastError);
    MQTTClient_destroy(&Client);
    Client = nullptr;
    return false;
  }

  bIsConnected = true;
  LastError = "";
  UE_LOG(
      LogTemp,
      Log,
      TEXT("Successfully connected to MQTT broker: %s"),
      *BrokerAddress
  );
  return true;
}

bool AMQTTActor::DisconnectFromMQTTBroker(int32 TimeoutMs)
{
  if (Client == nullptr)
  {
    return false;
  }

  MQTTClient mqttClient = (MQTTClient)Client;
  MQTTClient_disconnect(mqttClient, TimeoutMs);
  MQTTClient_destroy(&mqttClient);
  Client = nullptr;
  bIsConnected = false;

  UE_LOG(LogTemp, Log, TEXT("Disconnected from MQTT broker"));
  return true;
}

bool AMQTTActor::IsMQTTConnected()
{
  return bIsConnected && (Client != nullptr) && MQTTClient_isConnected(Client);
}

bool AMQTTActor::PublishMQTTMessage(
    const FString& Topic,
    const FString& Message,
    EMQTTQoS QoS,
    bool bRetain
)
{
  if (!IsMQTTConnected())
  {
    LastError = "Not connected to MQTT broker";
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LastError);
    return false;
  }

  std::string TopicStr = TCHAR_TO_UTF8(*Topic);
  std::string MessageStr = TCHAR_TO_UTF8(*Message);

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  pubmsg.payload = (void*)MessageStr.c_str();
  pubmsg.payloadlen = MessageStr.length();
  pubmsg.qos = static_cast<int>(QoS);
  pubmsg.retained = bRetain ? 1 : 0;

  MQTTClient_deliveryToken token;
  MQTTClient mqttClient = (MQTTClient)Client;
  int rc =
      MQTTClient_publishMessage(mqttClient, TopicStr.c_str(), &pubmsg, &token);

  if (rc != MQTTCLIENT_SUCCESS)
  {
    LastError = FString::Printf(TEXT("Failed to publish message: %d"), rc);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LastError);
    return false;
  }

  // Wait for delivery (optional, adjust timeout as needed)
  rc = MQTTClient_waitForCompletion(Client, token, 1000);

  UE_LOG(LogTemp, Log, TEXT("Published to %s: %s"), *Topic, *Message);
  return rc == MQTTCLIENT_SUCCESS;
}

bool AMQTTActor::SubscribeToMQTTTopic(const FString& Topic, EMQTTQoS QoS)
{
  if (!IsMQTTConnected())
  {
    LastError = "Not connected to MQTT broker";
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LastError);
    return false;
  }

  std::string TopicStr = TCHAR_TO_UTF8(*Topic);
  MQTTClient mqttClient = (MQTTClient)Client;
  int rc =
      MQTTClient_subscribe(mqttClient, TopicStr.c_str(), static_cast<int>(QoS));

  if (rc != MQTTCLIENT_SUCCESS)
  {
    LastError = FString::Printf(TEXT("Failed to subscribe to topic: %d"), rc);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LastError);
    return false;
  }

  UE_LOG(LogTemp, Log, TEXT("Subscribed to topic: %s"), *Topic);
  return true;
}

bool AMQTTActor::UnsubscribeFromMQTTTopic(const FString& Topic)
{
  if (!IsMQTTConnected())
  {
    LastError = "Not connected to MQTT broker";
    return false;
  }

  std::string TopicStr = TCHAR_TO_UTF8(*Topic);
  int rc = MQTTClient_unsubscribe(Client, TopicStr.c_str());

  if (rc != MQTTCLIENT_SUCCESS)
  {
    LastError =
        FString::Printf(TEXT("Failed to unsubscribe from topic: %d"), rc);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LastError);
    return false;
  }

  UE_LOG(LogTemp, Log, TEXT("Unsubscribed from topic: %s"), *Topic);
  return true;
}

bool AMQTTActor::HasMQTTMessages()
{
  FScopeLock Lock(&MessageQueueLock);
  return MessageQueue.Num() > 0;
}

bool AMQTTActor::GetNextMQTTMessage(FString& OutTopic, FString& OutMessage)
{
  FScopeLock Lock(&MessageQueueLock);

  if (MessageQueue.Num() == 0)
  {
    return false;
  }

  FMQTTMessage Msg = MessageQueue[0];
  MessageQueue.RemoveAt(0);

  OutTopic = Msg.Topic;
  OutMessage = Msg.Payload;

  return true;
}

FString AMQTTActor::GetMQTTLastError() { return LastError; }

// Callback implementations
void AMQTTActor::OnConnectionLost(void* Context, char* Cause)
{
  bIsConnected = false;
  FString CauseStr = FString(UTF8_TO_TCHAR(Cause));
  LastError = FString::Printf(TEXT("Connection lost: %s"), *CauseStr);
  UE_LOG(LogTemp, Warning, TEXT("%s"), *LastError);
}

int AMQTTActor::OnMessageArrived(
    void* Context,
    char* TopicName,
    int TopicLen,
    void* Message
)
{
  MQTTClient_message* mqttMsg = reinterpret_cast<MQTTClient_message*>(Message);

  // Message payload as FString
  FString Payload;
  if (mqttMsg->payload != nullptr && mqttMsg->payloadlen > 0)
  {
    char* PayloadStr = new char[mqttMsg->payloadlen + 1];
    memcpy(PayloadStr, mqttMsg->payload, mqttMsg->payloadlen);
    PayloadStr[mqttMsg->payloadlen] = '\0';

    Payload = FString(UTF8_TO_TCHAR(PayloadStr));
    delete[] PayloadStr;
  }

  FString Topic = FString(UTF8_TO_TCHAR(TopicName));

  // Add to message queue
  {
    FScopeLock Lock(&MessageQueueLock);
    FMQTTMessage NewMsg;
    NewMsg.Topic = Topic;
    NewMsg.Payload = Payload;
    MessageQueue.Add(NewMsg);
  }

  UE_LOG(LogTemp, Log, TEXT("Message received on %s: %s"), *Topic, *Payload);

  MQTTClient_freeMessage(&mqttMsg);
  MQTTClient_free(TopicName);

  return 1; // handled
}

void AMQTTActor::OnDeliveryComplete(void* Context, int Token)
{
  UE_LOG(LogTemp, Verbose, TEXT("Message delivery complete, token: %d"), Token);
}
