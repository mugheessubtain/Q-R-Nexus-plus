import { NextRequest, NextResponse } from 'next/server';

export async function POST(req: NextRequest) {
  try {
    const { message } = await req.json();
    if (!message) return NextResponse.json({ error: 'No message provided' }, { status: 400 });

    const GEMINI_API_KEY = process.env.GEMINI_API_KEY || "";
    if (!GEMINI_API_KEY) return NextResponse.json({ error: 'Gemini API key missing' }, { status: 500 });

    // 1. **CORRECTION:** Use the correct endpoint for `generateContent`
    const apiUrl = `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=${GEMINI_API_KEY}`;

    const res = await fetch(
      apiUrl,
      {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          // 2. **CORRECTION:** Removed 'Authorization' header. API key is now in the URL query parameter.
        },
        
        body: JSON.stringify({
          contents: [
            {
              role: 'user',
              parts: [{ text: message }],
            },
          ],
          // 💡 Use the correct field name for model configuration
          generationConfig: {
            temperature: 0.7,
          },
        }),
      }
    );

    if (!res.ok) {
      const text = await res.text();
      console.error('Gemini API error:', res.status, text);
      return NextResponse.json({ error: 'Gemini API error', details: text }, { status: 500 });
    }

    const data = await res.json();

    // 5. **CORRECTION:** The response structure for `generateContent` uses `candidates[0].content.parts[0].text`.
    const reply =
      data?.candidates?.[0]?.content?.parts?.[0]?.text ||
      'No response from Gemini';

    return NextResponse.json({ reply, raw: data });

  } catch (err) {
    console.error('Server error in /api/chat:', err);
    return NextResponse.json({ error: 'Internal server error', details: String(err) }, { status: 500 });
  }
}