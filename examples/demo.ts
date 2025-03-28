import SpeechRecognizer from "../lib";

async function main() {
  const recognizer = new SpeechRecognizer();
  
  try {
    console.log('Say something...');
    const text = await recognizer.recognizeAsync();
    console.log('Recognized:', text);
  } catch (error) {
    console.error('Error:', error);
  } finally {
    recognizer.destroy();
  }
}

main();