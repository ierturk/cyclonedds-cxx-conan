#include <dds/dds.hpp>
#include "calculator.hpp"
#include <iostream>
#include <thread>
#include <string>

using namespace dds::domain;
using namespace dds::topic;
using namespace dds::pub;
using namespace dds::sub;

class ReplyListener : public NoOpDataReaderListener<calculator::Reply> {
public:
    void on_data_available(DataReader<calculator::Reply>& reader) override {
        LoanedSamples<calculator::Reply> samples = reader.take();
        for (const auto& sample : samples) {
            if (sample.info().valid()) {
                switch (sample.data()._d()) {
                    case calculator::ReplyType::ADD_REPLY: {
                        const calculator::AddReply& reply = sample.data().add_reply();
                        std::cout << "Client received add reply: " << reply.result() << std::endl;
                        break;
                    }
                    case calculator::ReplyType::SUBTRACT_REPLY: {
                        const calculator::SubtractReply& reply = sample.data().subtract_reply();
                        std::cout << "Client received subtract reply: " << reply.result() << std::endl;
                        break;
                    }
                    default:
                        std::cerr << "Unknown reply type" << std::endl;
                        break;
                }
            }
        }
    }
};

int main() {
    DomainParticipant participant(0);
    Topic<calculator::Request> request_topic(participant, "Request");
    Topic<calculator::Reply> reply_topic(participant, "Reply");

    Publisher publisher(participant);
    DataWriter<calculator::Request> request_writer(publisher, request_topic);

    Subscriber subscriber(participant);
    ReplyListener listener;
    DataReader<calculator::Reply> reply_reader(subscriber, reply_topic);
    reply_reader.listener(&listener, dds::core::status::StatusMask::data_available());

    std::string operation;
    double operand1, operand2;
    while (true) {
        std::cout << "Enter operation (add, subtract) or 'exit' to quit: ";
        std::cin >> operation;
        if (operation == "exit") {
            break;
        }
        if (operation != "add" && operation != "subtract") {
            std::cout << "Invalid operation. Please try again." << std::endl;
            continue;
        }
        std::cout << "Enter operand1: ";
        std::cin >> operand1;
        std::cout << "Enter operand2: ";
        std::cin >> operand2;

        calculator::Request request;
        if (operation == "add") {
            request._d(calculator::RequestType::ADD_REQUEST);
            calculator::AddRequest add_request;
            add_request.id("client1");
            add_request.operand1(operand1);
            add_request.operand2(operand2);
            request.add_request(add_request);
            request_writer.write(request);
            std::cout << "Client sent add request: " << operand1 << " " << operand2 << std::endl;
        } else if (operation == "subtract") {
            calculator::SubtractRequest subtract_request;
            subtract_request.id("client1");
            subtract_request.operand1(operand1);
            subtract_request.operand2(operand2);
            request.subtract_request(subtract_request);
            request_writer.write(request);
            std::cout << "Client sent subtract request: " << operand1 << " " << operand2 << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}