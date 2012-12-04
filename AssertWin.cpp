#include "PlatformPch.h"
#include "Platform/System.h"
#include "Platform/Assert.h"
#include "Platform/Console.h"
#include "Platform/Encoding.h"

#if HELIUM_OS_WIN

using namespace Helium;

/// Terminate the application on a fatal error.
///
/// @param[in] exitCode  Error code associated with the exit.
void Helium::FatalExit( int exitCode )
{
	::FatalExit( exitCode );
}

#if HELIUM_ASSERT_ENABLED

/// Handle an assertion.
///
/// @param[in] pMessageText  Assert message text.
AssertResult Assert::TriggerImplementation( const tchar_t* pMessageText )
{
	tchar_t messageBoxText[ 1024 ];
	StringPrint(
		messageBoxText,
		( TXT( "%s\n\nChoose \"Abort\" to terminate the program, \"Retry\" to debug the program (if a debugger " )
		  TXT( "is attached), or \"Ignore\" to attempt to skip over the error." ) ),
		pMessageText );

	HELIUM_TCHAR_TO_WIDE( messageBoxText, wideMessageBoxText );
	int result = MessageBoxW( NULL, wideMessageBoxText, L"Assert", MB_ABORTRETRYIGNORE );

	return ( result == IDABORT ? AssertResults::Abort : ( result == IDIGNORE ? AssertResults::Continue : AssertResults::Break ) );
}

#endif  // HELIUM_OS_WIN

#endif  // HELIUM_ASSERT_ENABLED
