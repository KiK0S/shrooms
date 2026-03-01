#pragma once

#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#include <SDL2/SDL.h>
#endif

namespace share_bridge {

enum class ShareRoute {
  WebShare,
  Clipboard,
  Unsupported,
};

inline ShareRoute share_or_copy_score_text(const std::string& text) {
#ifdef __EMSCRIPTEN__
  const int route = EM_ASM_INT({
    const value = UTF8ToString($0);
    const nav = (typeof navigator !== 'undefined') ? navigator : null;

    const copyFallback = () => {
      if (typeof document === 'undefined') {
        return 0;
      }
      const ta = document.createElement('textarea');
      ta.value = value;
      ta.style.position = 'fixed';
      ta.style.opacity = '0';
      ta.style.pointerEvents = 'none';
      document.body.appendChild(ta);
      ta.focus();
      ta.select();
      let ok = false;
      try {
        ok = document.execCommand('copy');
      } catch (e) {
        ok = false;
      }
      document.body.removeChild(ta);
      return ok ? 1 : 0;
    };

    const copyClipboard = () => {
      if (!nav || !nav.clipboard || !nav.clipboard.writeText) {
        return copyFallback();
      }
      try {
        nav.clipboard.writeText(value).catch(() => {
          copyFallback();
        });
        return 1;
      } catch (e) {
        return copyFallback();
      }
    };

    const ua = (nav && nav.userAgent) ? nav.userAgent : "";
    const isMobile = /Android|iPhone|iPad|iPod|Mobi/i.test(ua);
    if (isMobile && nav && typeof nav.share === 'function') {
      try {
        nav.share({ text: value }).catch(() => {
          copyClipboard();
        });
        return 2;
      } catch (e) {
        return copyClipboard();
      }
    }

    return copyClipboard();
  }, text.c_str());

  if (route == 2) {
    return ShareRoute::WebShare;
  }
  if (route == 1) {
    return ShareRoute::Clipboard;
  }
  return ShareRoute::Unsupported;
#else
  if (SDL_SetClipboardText(text.c_str()) == 0) {
    return ShareRoute::Clipboard;
  }
  return ShareRoute::Unsupported;
#endif
}

}  // namespace share_bridge
