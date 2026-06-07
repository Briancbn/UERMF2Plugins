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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MQTTActor.generated.h"

UENUM(BlueprintType)
enum class EMQTTConnectionStatus : uint8
{
    Disconnected    UMETA(DisplayName = "Disconnected"),
    Connecting      UMETA(DisplayName = "Connecting"),
    Connected       UMETA(DisplayName = "Connected"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EMQTTQoS : uint8
{
    AtMostOnce      UMETA(DisplayName = "At Most Once (QoS 0)"),
    AtLeastOnce     UMETA(DisplayName = "At Least Once (QoS 1)"),
    ExactlyOnce     UMETA(DisplayName = "Exactly Once (QoS 2)")
};

UCLASS()
class RMF2RUNTIME_API AMQTTActor : public AActor
{
	GENERATED_BODY()

public:
	/// \brief Sets default values for this actor's properties
	AMQTTActor();

	/// \brief Called every frame
	virtual void Tick(float DeltaTime) override;

     /// \brief Create and connect to an MQTT broker
     /// \param BrokerAddress - MQTT broker address (e.g., "tcp://localhost:1883")
     /// \param ClientID - Unique client identifier
     /// \param Username - Username for authentication (leave empty if not needed)
     /// \param Password - Password for authentication (leave empty if not needed)
     /// \param KeepAliveInterval - Keep alive interval in seconds (default 60)
     /// \param CleanSession - Whether to start a clean session (default true)
     /// \return True if connection was initiated successfully
    UFUNCTION(BlueprintCallable, Category = "MQTT|Connection",
        meta = (DisplayName = "Connect to MQTT Broker", Keywords = "mqtt connect broker"))
    static bool ConnectToMQTTBroker(
        const FString& BrokerAddress,
        const FString& ClientID,
        const FString& Username = "",
        const FString& Password = "",
        int32 KeepAliveInterval = 60,
        bool bCleanSession = true
    );

    /// \brief Disconnect from the MQTT broker
    /// \param TimeoutMs - Timeout in milliseconds for disconnection
    /// \return True if disconnected successfully
    UFUNCTION(BlueprintCallable, Category = "MQTT|Connection",
        meta = (DisplayName = "Disconnect from MQTT Broker", Keywords = "mqtt disconnect"))
    static bool DisconnectFromMQTTBroker(int32 TimeoutMs = 1000);

    /// \brief Check if connected to MQTT broker
    /// \return True if connected
    UFUNCTION(BlueprintPure, Category = "MQTT|Connection",
        meta = (DisplayName = "Is MQTT Connected", Keywords = "mqtt connected status"))
    static bool IsMQTTConnected();

    /// \brief Publish a message to an MQTT topic
    /// \param Topic - Topic to publish to
    /// \param Message - Message payload
    /// \param QoS - Quality of Service level
    /// \param bRetain - Whether the broker should retain this message
    /// \return True if message was sent successfully
    UFUNCTION(BlueprintCallable, Category = "MQTT|Messaging",
        meta = (DisplayName = "Publish MQTT Message", Keywords = "mqtt publish send message"))
    static bool PublishMQTTMessage(
        const FString& Topic,
        const FString& Message,
        EMQTTQoS QoS = EMQTTQoS::AtMostOnce,
        bool bRetain = false
    );

    /// \brief Subscribe to an MQTT topic
    /// \param Topic - Topic to subscribe to (can include wildcards like "home/+/temperature")
    /// \param QoS - Quality of Service level
    /// \return True if subscription was successful
    UFUNCTION(BlueprintCallable, Category = "MQTT|Messaging",
        meta = (DisplayName = "Subscribe to MQTT Topic", Keywords = "mqtt subscribe topic"))
    static bool SubscribeToMQTTTopic(
        const FString& Topic,
        EMQTTQoS QoS = EMQTTQoS::AtMostOnce
    );

    /// \brief Unsubscribe from an MQTT topic
    /// \param Topic - Topic to unsubscribe from
    /// \return True if unsubscription was successful
    UFUNCTION(BlueprintCallable, Category = "MQTT|Messaging",
        meta = (DisplayName = "Unsubscribe from MQTT Topic", Keywords = "mqtt unsubscribe"))
    static bool UnsubscribeFromMQTTTopic(const FString& Topic);


    /// \brief Check if there are any pending MQTT messages
    /// \return True if messages are available
    UFUNCTION(BlueprintPure, Category = "MQTT|Messaging",
        meta = (DisplayName = "Has MQTT Messages", Keywords = "mqtt message pending"))
    static bool HasMQTTMessages();

    /// \brief Get the next received MQTT message
    /// \param OutTopic - The topic the message was received on
    /// \param OutMessage - The message payload
    /// \return True if a message was retrieved
    UFUNCTION(BlueprintCallable, Category = "MQTT|Messaging",
        meta = (DisplayName = "Get Next MQTT Message", Keywords = "mqtt receive message"))
    static bool GetNextMQTTMessage(FString& OutTopic, FString& OutMessage);


    /// \brief Get the last MQTT error message
    /// \return Error message string
    UFUNCTION(BlueprintPure, Category = "MQTT|Utilities",
        meta = (DisplayName = "Get MQTT Last Error", Keywords = "mqtt error"))
    static FString GetMQTTLastError();

    /// \brief Callback invoked when an MQTT message arrives.
    /// \param Context User-defined context passed to the MQTT client.
    /// \param TopicName Pointer to the topic string (not null-terminated).
    /// \param TopicLen Length of the topic string.
    /// \param Message Pointer to the received MQTT message.
    /// \return Always return 1 to indicate successful message handling.
    static int OnMessageArrived(void* Context, char* TopicName, int TopicLen, void* Message);

protected:
	/// \brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
    /// \brief Static client handle
    static void* Client;

    /// \brief Client connection status
    static bool bIsConnected;

    /// \brief Latest error encountered by client
    static FString LastError;

    /// \struct MQTT Message/Payload
    /// \brief MQTT Message/Payload
    struct FMQTTMessage
    {
        FString Topic;
        FString Payload;
    };

    /// \brief Message queue for received messages
    static TArray<FMQTTMessage> MessageQueue;

    /// \brief Synchronizes access to the message queue.
    static FCriticalSection MessageQueueLock;

    /// \brief Callback invoked when the MQTT connection is lost.
    /// \param Context User-defined context passed to the MQTT client.
    /// \param Cause Null-terminated string describing the reason for the disconnect.
    static void OnConnectionLost(void* Context, char* Cause);

    /// \brief Callback invoked when message delivery is confirmed by the broker.
    /// \param Context User-defined context passed to the MQTT client.
    /// \param Token Delivery token associated with the published message.
    static void OnDeliveryComplete(void* Context, int Token);
};
