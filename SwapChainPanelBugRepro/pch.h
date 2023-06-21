#pragma once

// Suppresses the "min" and "max" macros which are defined in
// windef.h. Instead std::min and std::max which are defined
// in <algorithm> shall be used.
#define NOMINMAX

#include <d2d1_2.h>
#include <d3d11_2.h>
#include <windows.h>

// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime

#include <microsoft.ui.xaml.media.dxinterop.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
