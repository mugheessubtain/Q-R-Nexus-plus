// pages/api/ai/predict/[symbol].ts or app/api/ai/predict/[symbol]/route.ts
import { NextRequest, NextResponse } from 'next/server';

// Define the handler function (POST method based on your fetch)
export async function POST(req: NextRequest, { params }: { params: { symbol: string } }) {
  try {
    const symbol = params.symbol;

    // The frontend sends the chartData as a stringified JSON object
    const { data: chartDataString } = await req.json();
    
    if (!chartDataString) {
      return NextResponse.json({ error: 'No chart data provided' }, { status: 400 });
    }

    const GEMINI_API_KEY = process.env.GEMINI_API_KEY || "";
    if (!GEMINI_API_KEY) {
      return NextResponse.json({ error: 'Gemini API key missing' }, { status: 500 });
    }

    // --- Gemini API Setup ---
    const apiUrl = `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=${GEMINI_API_KEY}`;
    
    const systemPrompt = 
      "You are a professional crypto & stock market analyst. " +
      "You specialize in market structure, technical analysis, execution models, " +
      "and interpreting candle data. " +
      "Always provide concise, accurate, expert-level responses, focusing on the next 1-4 hour trend. " +
      "Never hallucinate missing information. " +
      "If something is unclear or unknown, say it directly. " +
      "Keep your tone analytical and objective.";

    // Construct the specific prompt for market prediction
    const userMessage = `Analyze the following time-series chart data for ${symbol}. 
The data is an array of objects where 'time' is the timestamp, 'price' is the closing price, and 'volume' is the trade volume. 
Based ONLY on this data, provide a professional analysis, potential next price target, and a clear prediction (Bullish, Bearish, or Consolidation) for the next short-term period (1-4 hours).
\n\nCHART DATA: ${chartDataString}`;
    // --- End Gemini API Setup ---

    const res = await fetch(
      apiUrl,
      {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          config: {
            systemInstruction: systemPrompt,
          },
          contents: [
            {
              role: 'user',
              parts: [{ text: userMessage }],
            },
          ],
          generationConfig: {
            temperature: 0.1, // Using a lower temperature for technical prediction
          },
        }),
      }
    );

    if (!res.ok) {
      const text = await res.text();
      console.error('Gemini Prediction API error:', res.status, text);
      return NextResponse.json({ error: 'Gemini API error', details: text }, { status: res.status });
    }

    const data = await res.json();
    
    // The frontend expects the raw Gemini response structure, so we return it directly.
    return NextResponse.json(data); 

  } catch (err) {
    console.error('Server error in /api/ai/predict:', err);
    return NextResponse.json({ error: 'Internal server error', details: String(err) }, { status: 500 });
  }
}