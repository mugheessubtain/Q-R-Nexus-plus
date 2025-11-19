// "use client";
// import { useState } from "react";

// export default function Home() {
//   const [name, setName] = useState("");
//   const [response, setResponse] = useState("");

//   const getMessage = async () => {
//     const res = await fetch("http://localhost:8080/api/hello");
//     const data = await res.json();
//     setResponse(data);
//   };

//   const sendData = async () => {
//     const res = await fetch("http://localhost:8080/api/data", {
//       method: "POST",
//       headers: { "Content-Type": "application/json" },
//       body: JSON.stringify({ name }),
//     });
//     const data = await res.json();
//     setResponse(data.reply);
//   };

//   return (
//     <div style={{ margin: "50px" }}>
//       <h1>Next.js + Drogon C++ Backend</h1>

//       <button onClick={getMessage}>Fetch GET</button>

//       <br /><br />

//       <input value={name} onChange={(e)=>setName(e.target.value)} placeholder="your name" />
//       <button onClick={sendData}>Send Name</button>

//       <h3>{response}</h3>
//     </div>
//   );
// }















'use client';

import { useState, useEffect } from 'react';
import Link from 'next/link';

export default function LandingPage() {
  const [apiResponse, setApiResponse] = useState<string>('');

  useEffect(() => {
    // Add your API connection logic here
    // Example:
    // fetch('/api/test')
    //   .then(res => res.json())
    //   .then(data => setApiResponse(data.message))
    //   .catch(err => setApiResponse('Connection failed'));
  }, []);

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-purple-900 to-slate-900 relative overflow-hidden">
      {/* Background effects */}
      <div className="absolute inset-0 bg-[radial-gradient(circle_at_50%_50%,rgba(120,119,198,0.1),transparent_50%)]" />
      
      <main className="relative min-h-screen flex items-center justify-center px-4 py-12">
        <div className="max-w-4xl w-full text-center space-y-8 animate-fade-in">
          {/* Main heading */}
          <h1 className="text-5xl md:text-7xl font-bold text-white leading-tight">
            The Only Trading Platform
            <br />
            <span className="bg-gradient-to-r from-emerald-400 to-cyan-400 bg-clip-text text-transparent">
              with Built-in Predictions
            </span>
          </h1>

          {/* CTA Button */}
          <Link 
            href="/signup" 
            className="inline-flex items-center gap-3 px-8 py-4 bg-gradient-to-r from-emerald-500 to-cyan-500 text-white font-semibold rounded-lg hover:shadow-[0_0_30px_rgba(16,185,129,0.5)] transition-all duration-300 hover:scale-105 group"
          >
            JOIN NOW
            <svg 
              width="24" 
              height="24" 
              viewBox="0 0 24 24" 
              fill="none" 
              className="group-hover:translate-x-1 transition-transform"
            >
              <path 
                d="M5 12H19" 
                stroke="currentColor" 
                strokeWidth="2" 
                strokeLinecap="round" 
                strokeLinejoin="round"
              />
              <path 
                d="M12 5L19 12L12 19" 
                stroke="currentColor" 
                strokeWidth="2" 
                strokeLinecap="round" 
                strokeLinejoin="round"
              />
            </svg>
          </Link>

          {/* Secondary content */}
          <div className="mt-16 space-y-4">
            <h2 className="text-3xl md:text-4xl font-bold text-white">
              Welcome to Q&R NEXUS
            </h2>
            <p className="text-gray-300 text-lg">
              {/* Testing backend connection... */}
            </p>
            {apiResponse && (
              <div className="mt-4 text-emerald-400 font-bold">
                {apiResponse}
              </div>
            )}
          </div>
        </div>
      </main>

      {/* Q&R Bubble */}
      <div className="fixed bottom-8 right-8 w-16 h-16 bg-gradient-to-br from-emerald-500 to-cyan-500 rounded-full flex items-center justify-center text-white font-bold shadow-lg hover:shadow-[0_0_30px_rgba(16,185,129,0.5)] transition-all duration-300 hover:scale-110 cursor-pointer z-50">
        <span className="text-sm">Q&R</span>
      </div>

      <style jsx>{`
        @keyframes fade-in {
          from {
            opacity: 0;
            transform: translateY(20px);
          }
          to {
            opacity: 1;
            transform: translateY(0);
          }
        }
        .animate-fade-in {
          animation: fade-in 0.8s ease-out;
        }
      `}</style>
    </div>
  );
}