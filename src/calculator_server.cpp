#include <dds/dds.hpp>
#include "calculator.hpp"
#include <iostream>
#include <thread>
#include <future>
#include <limits>

using namespace dds::domain;
using namespace dds::topic;
using namespace dds::pub;
using namespace dds::sub;

double perform_operation(const std::string& operation, double operand1, double operand2) {
    if (operation == "add") {
        return operand1 + operand2;
    } else if (operation == "subtract") {
        return operand1 - operand2;
    } else {
        throw std::runtime_error("Unknown operation");
    }
}

void process_request(const calculator::Request& request, DataWriter<calculator::Reply>& reply_writer) {
    calculator::Reply reply;
    switch (request._d()) {
        case calculator::RequestType::ADD_REQUEST: {
            const calculator::AddRequest& add_request = request.add_request();
            std::cout << "Server received add request: " << add_request.operand1() << " " << add_request.operand2() << std::endl;
            reply._d(calculator::ReplyType::ADD_REPLY);
            reply.add_reply().id(add_request.id());
            try {
                reply.add_reply().result(perform_operation("add", add_request.operand1(), add_request.operand2()));
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                reply.add_reply().result(std::numeric_limits<double>::quiet_NaN());
            }
            break;
        }
        case calculator::RequestType::SUBTRACT_REQUEST: {
            const calculator::SubtractRequest& subtract_request = request.subtract_request();
            std::cout << "Server received subtract request: " << subtract_request.operand1() << " " << subtract_request.operand2() << std::endl;
            // reply._d(calculator::ReplyType::SUBTRACT_REPLY);
            calculator::SubtractReply subtract_reply;
            subtract_reply.id(subtract_request.id());
            reply.subtract_reply(subtract_reply);
            try {
                reply.subtract_reply().result(perform_operation("subtract", subtract_request.operand1(), subtract_request.operand2()));
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                reply.subtract_reply().result(std::numeric_limits<double>::quiet_NaN());
            }
            break;
        }
        default:
            std::cerr << "Unknown request type" << std::endl;
            return;
    }
    reply_writer.write(reply);
    std::cout << "Server sent reply" << std::endl;
}

class RequestListener : public NoOpDataReaderListener<calculator::Request> {
public:
    RequestListener(DataWriter<calculator::Reply>& writer) : reply_writer(writer) {}

    void on_data_available(DataReader<calculator::Request>& reader) override {
        LoanedSamples<calculator::Request> samples = reader.take();
        for (const auto& sample : samples) {
            if (sample.info().valid()) {
                std::async(std::launch::async, process_request, sample.data(), std::ref(reply_writer));
            }
        }
    }

private:
    DataWriter<calculator::Reply>& reply_writer;
};

int main() {
    DomainParticipant participant(0);
    Topic<calculator::Request> request_topic(participant, "Request");
    Topic<calculator::Reply> reply_topic(participant, "Reply");

    Publisher publisher(participant);
    DataWriter<calculator::Reply> reply_writer(publisher, reply_topic);

    Subscriber subscriber(participant);
    DataReader<calculator::Request> request_reader(subscriber, request_topic);

    RequestListener listener(reply_writer);
    request_reader.listener(&listener, dds::core::status::StatusMask::data_available());

    // Keep the main thread alive
    std::cout << "Server is running. Press Ctrl+C to exit." << std::endl;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}