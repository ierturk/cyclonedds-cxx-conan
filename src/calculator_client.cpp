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
                std::cout << "Client received reply: " << sample.data().result() << std::endl;
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
        std::cout << "Enter operation (add, subtract, multiply, divide): ";
        std::cin >> operation;
        std::cout << "Enter operand1: ";
        std::cin >> operand1;
        std::cout << "Enter operand2: ";
        std::cin >> operand2;

        calculator::Request request;
        request.id("client1");
        request.operation(operation);
        request.operand1(operand1);
        request.operand2(operand2);
        request_writer.write(request);
        std::cout << "Client sent request: " << operation << " " << operand1 << " " << operand2 << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}