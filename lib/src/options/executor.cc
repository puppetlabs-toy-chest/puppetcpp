#include <puppet/options/executor.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace options {

    executor::executor(options::command const& command, callback_type callback) :
        _command(command),
        _callback(rvalue_cast(callback))
    {
    }

    options::command const& executor::command() const
    {
        return _command;
    }

    int executor::execute() const
    {
        return _callback();
    }

}}  // namespace puppet::options
