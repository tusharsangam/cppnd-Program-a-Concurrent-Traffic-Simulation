#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
     // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg; // will not be copied due to return value optimization 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one(); // notify client after pushing new Vehicle into vector

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        auto signal = _messagequeue.receive();
        if (signal == TrafficLightPhase::green)
            break;     
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::unique_lock <std::mutex> u(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    int upper = 6;
    int lower = 4;
    int cycleduration;
    std::chrono::system_clock::time_point cyclestart;
    std::chrono::system_clock::time_point cycleend;
    int difference;
    TrafficLightPhase temp;
    while (true) {
        std::srand(time(0));
        cycleduration = (std::rand() % (upper - lower + 1)) + lower;
        cyclestart = std::chrono::system_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(cycleduration));
        cycleend = std::chrono::system_clock::now();
        difference = std::chrono::duration_cast<std::chrono::seconds>(cycleend - cyclestart).count();
        if (difference >= cycleduration) {
            std::lock_guard<std::mutex> lck(_mutex);
            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;  
            temp = _currentPhase;
            _messagequeue.send(std::move(temp));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); //mandatory break
    }
}

//TrafficLight::TrafficLight() {}
//TrafficLight::~TrafficLight() {}
