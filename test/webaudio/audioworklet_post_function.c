#include <emscripten/webaudio.h>
#include <assert.h>

// This test showcases posting messages (function calls) between the main thread
// and the Audio Worklet thread using the
// emscripten_audio_worklet_post_function_*() API.

// This event will fire on the main thread.
void MessageReceivedOnMainThread(int d, int e, int f) {
  emscripten_outf("MessageReceivedOnMainThread: d=%d, e=%d, f=%d", d, e, f);
  assert(!emscripten_current_thread_is_audio_worklet());
  assert(d == 1 && e == 2 && f == 3);

  // test succeeded, were able to post a message from main thread to audio thread and back!
  emscripten_force_exit(0);
}

// This event will fire on the audio worklet thread.
void MessageReceivedInAudioWorkletThread(int a, int b) {
  emscripten_outf("MessageReceivedInAudioWorkletThread: a=%d, b=%d", a, b);
  assert(emscripten_current_thread_is_audio_worklet());
  assert(a == 42 && b == 9000);
  emscripten_audio_worklet_post_function_viii(EMSCRIPTEN_AUDIO_MAIN_THREAD, MessageReceivedOnMainThread, /*d=*/1, /*e=*/2, /*f=*/3);
}

// This callback will fire when the audio worklet thread has been initialized.
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  emscripten_out("WebAudioWorkletThreadInitialized");
  emscripten_audio_worklet_post_function_vii(audioContext, MessageReceivedInAudioWorkletThread, /*a=*/42, /*b=*/9000);
}

uint8_t wasmAudioWorkletStack[4096];

int main() {
  // Create an audio context
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0 /* use default constructor options */);

  // and kick off Audio Worklet scope initialization, which shares the Wasm
  // Module and Memory to the AudioWorklet scope and initializes its stack.
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);

  emscripten_exit_with_live_runtime();
}
