#include <dds/dds.hpp>
#include "calculator.hpp"
#include <iostream>
#include <thread>

using namespace dds::domain;
using namespace dds::topic;
using namespace dds::pub;
using namespace dds::sub;

double perform_operation(const std::string& operation, double operand1, double operand2) {
    if (operation == "add") {
        return operand1 + operand2;
    } else if (operation == "subtract") {
        return operand1 - operand2;
    } else if (operation == "multiply") {
        return operand1 * operand2;
    } else if (operation == "divide") {
        if (operand2 != 0) {
            return operand1 / operand2;
        } else {
            throw std::runtime_error("Division by zero");
        }
    } else {
        throw std::runtime_error("Unknown operation");
    }
}

int main() {
    DomainParticipant participant(0);
    Topic<calculator::Request> request_topic(participant, "Request");
    Topic<calculator::Reply> reply_topic(participant, "Reply");

    Publisher publisher(participant);
    DataWriter<calculator::Reply> reply_writer(publisher, reply_topic);

    Subscriber subscriber(participant);
    DataReader<calculator::Request> request_reader(subscriber, request_topic);

    while (true) {
        LoanedSamples<calculator::Request> samples = request_reader.take();
        for (const auto& sample : samples) {
            if (sample.info().valid()) {
                std::cout << "Server received request: " << sample.data().operation() << " "
                          << sample.data().operand1() << " " << sample.data().operand2() << std::endl;

                // Process the request and send a reply
                calculator::Reply reply;
                reply.id(sample.data().id());
                try {
                    reply.result(perform_operation(sample.data().operation(), sample.data().operand1(), sample.data().operand2()));
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    reply.result(std::numeric_limits<double>::quiet_NaN());
                }
                reply_writer.write(reply);
                std::cout << "Server sent reply: " << reply.result() << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}