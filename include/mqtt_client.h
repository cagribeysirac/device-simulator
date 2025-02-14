// Copyright 2025 Cagribey

#ifndef INCLUDE_MQTT_CLIENT_H_
#define INCLUDE_MQTT_CLIENT_H_

#include <chrono>
#include <iostream>
#include <string>

#include "mqtt/client.h"

class MqttClient : public mqtt::callback {
 private:
    mqtt::async_client* async_client;
    mqtt::connect_options conn_opts;
    mqtt::message_ptr pubmsg;
    mqtt::delivery_token_ptr token;
    int rc;
    const std::string BROKER_ADDRESS;
    const std::string CLIENT_ID;

 public:
    MqttClient(const std::string& broker_address, const std::string& client_id)
        : BROKER_ADDRESS(broker_address), CLIENT_ID(client_id) {
        async_client = new mqtt::async_client(BROKER_ADDRESS, CLIENT_ID);
        async_client->set_callback(*this);
        conn_opts = mqtt::connect_options_builder().clean_session().finalize();
    }

    void connect() {
        try {
            async_client->connect(conn_opts)->wait();
        } catch (const mqtt::exception& exc) {
            std::cerr << "Error: " << exc.what() << std::endl;
        }
    }

    void disconnect() {
        try {
            async_client->disconnect()->wait();
        } catch (const mqtt::exception& exc) {
            std::cerr << "Error: " << exc.what() << std::endl;
        }
    }

    void subscribe(const std::string& topic, int qos = 0) {
        try {
            async_client->subscribe(topic, qos)->wait();
        } catch (const mqtt::exception& exc) {
            std::cerr << "Error: " << exc.what() << std::endl;
        }
    }

    void publish(const std::string& topic, const std::string& payload, int qos = 0,
                 bool retained = false) {
        try {
            auto pubmsg = mqtt::make_message(topic, payload);
            pubmsg->set_qos(qos);
            pubmsg->set_retained(retained);
            token = async_client->publish(pubmsg);

            // Wait for the message to be delivered
            token->wait_for(std::chrono::seconds(5));  // Add timeout to prevent hanging

            if (!token->is_complete()) {
                std::cerr << "Message delivery timeout" << std::endl;
            }
        } catch (const mqtt::exception& exc) {
            std::cerr << "Error in publish: " << exc.what() << std::endl;
        }
    }

    void connected(const std::string& cause) override {
        // Called when the client is connected to the broker
        std::cout << "Connected to broker: " << cause << std::endl;
    }

    void connection_lost(const std::string& cause) override {
        // Called when the client is disconnected from the broker
        std::cout << "Connection lost: " << cause << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        // Called when the client receives a message from the broker
        std::cout << "Message arrived: " << msg->to_string() << std::endl;
    }

    void delivery_complete([[maybe_unused]] mqtt::delivery_token_ptr token) override {
        // Called when the message is delivered successfully
        // This function is not be called while QOS is 0
        std::cout << "Message delivered successfully" << std::endl;
    }
};

#endif  // INCLUDE_MQTT_CLIENT_H_
