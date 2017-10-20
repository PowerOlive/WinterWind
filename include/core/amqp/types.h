#pragma once

#include <memory>
#include <functional>

namespace winterwind
{
namespace amqp
{

class Envelope;
typedef std::shared_ptr<Envelope> EnvelopePtr;
typedef std::function<bool(EnvelopePtr)> EnvelopeCallbackFct;

}
}
