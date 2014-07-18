#include <JoystickError.h>

#include <sstream>

#include <boost/exception/get_error_info.hpp>

using namespace boost;

const char* JoystickError::what() const throw() {
  const throw_function::value_type* const funcPtr  = get_error_info< throw_function >( *this );
  const throw_file::value_type*     const filePtr  = get_error_info< throw_file     >( *this );
  const throw_line::value_type*     const linePtr  = get_error_info< throw_line     >( *this );
  const NumberInfo::value_type*     const numPtr   = get_error_info< NumberInfo     >( *this );
  const SDLInfo::value_type*        const errorPtr = get_error_info< SDLInfo        >( *this );

  std::ostringstream msg;
  if( funcPtr && filePtr && linePtr )
    msg << *funcPtr << "[" << *filePtr << ":" << *linePtr << "] ";
  msg << "JoystickError - ";
  switch( mCause ) {
    case( Cause::NoSuchJoystick ): msg << "No such joystick ";
                                   break;
    case( Cause::InitError )     : msg << "Error initalizing SDL ";
                                   break;
    case( Cause::EventError )    : msg << "Error waiting for next joystick event ";
                                   break;
    default                      : msg << "unknown";
                                   break;
  }
  if( numPtr )
    msg << *numPtr;
  if( errorPtr )
    msg << *errorPtr;
  return msg.str().c_str();
}
