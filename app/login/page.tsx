'use client';

import { useState } from 'react';
import Link from 'next/link';
import { useRouter } from 'next/navigation';
import { Eye, EyeOff, ChevronDown } from 'lucide-react';

export default function LoginPage() {
  const router = useRouter();
  const [showPassword, setShowPassword] = useState(false);
  const [keepSignedIn, setKeepSignedIn] = useState(true);
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    // Add your login logic here
    router.push('/dashboard');
  };

  const handleScroll = () => {
    window.scrollBy({ top: window.innerHeight, behavior: 'smooth' });
  };

  return (
    <div className="min-h-screen bg-slate-50 flex items-center justify-center p-4">
      <div className="w-full max-w-7xl bg-white rounded-2xl shadow-2xl overflow-hidden flex flex-col lg:flex-row min-h-[600px]">
        
        {/* Left Side - About Us (Dark Panel) */}
        <div className="lg:w-1/2 bg-gradient-to-br from-slate-900 via-slate-800 to-slate-900 p-8 lg:p-12 flex flex-col justify-between text-white relative">
          <div className="space-y-6">
            <h2 className="text-3xl lg:text-4xl font-bold mb-6">About Us</h2>
            
            <p className="text-slate-300 leading-relaxed">
              At Q&R, we're redefining the way the world trades. Built by a team of experienced analysts, designers, and AI engineers, our platform combines advanced financial technology with intelligent market forecasting to help traders make faster, smarter, and more confident decisions.
            </p>
            
            <p className="text-slate-300 leading-relaxed">
              We understand that in today's markets, information moves faster than opportunity. That's why we've built a system that moves even faster.
            </p>
            
            <p className="text-slate-300 leading-relaxed">
              Our proprietary AI-driven prediction engine analyzes real-time data from stocks, forex, commodities, and cryptocurrencies, identifying emerging trends and forecasting market movements with remarkable accuracy.
            </p>
          </div>

          <button
            type="button"
            onClick={handleScroll}
            className="mt-8 inline-flex items-center gap-2 text-slate-300 hover:text-white transition-colors group"
          >
            Scroll
            <ChevronDown className="w-5 h-5 group-hover:translate-y-1 transition-transform" />
          </button>
        </div>

        {/* Right Side - Login Form (Light Panel) */}
        <div className="lg:w-1/2 bg-white p-8 lg:p-12 flex items-center justify-center">
          <form onSubmit={handleSubmit} className="w-full max-w-md space-y-6">
            <div className="space-y-2">
              <h3 className="text-3xl font-bold text-slate-900">Login</h3>
              <p className="text-slate-600">Enter your credentials to access your account</p>
            </div>

            {/* Email Field */}
            <div className="space-y-2">
              <label htmlFor="email" className="block text-sm font-medium text-slate-700">
                Email Address
              </label>
              <input
                type="email"
                id="email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                required
                className="w-full px-4 py-3 border border-slate-300 rounded-lg focus:ring-2 focus:ring-slate-900 focus:border-transparent outline-none transition-all text-slate-900"
                placeholder="your@email.com"
              />
            </div>

            {/* Password Field */}
            <div className="space-y-2">
              <label htmlFor="password" className="block text-sm font-medium text-slate-700">
                Password
              </label>
              <div className="relative">
                <input
                  type={showPassword ? 'text' : 'password'}
                  id="password"
                  value={password}
                  onChange={(e) => setPassword(e.target.value)}
                  required
                  className="w-full px-4 py-3 pr-12 border border-slate-300 rounded-lg focus:ring-2 focus:ring-slate-900 focus:border-transparent outline-none transition-all text-slate-900"
                  placeholder="••••••••"
                />
                <button
                  type="button"
                  onClick={() => setShowPassword(!showPassword)}
                  className="absolute right-3 top-1/2 -translate-y-1/2 text-slate-500 hover:text-slate-700 transition-colors"
                >
                  {showPassword ? (
                    <EyeOff className="w-5 h-5" />
                  ) : (
                    <Eye className="w-5 h-5" />
                  )}
                </button>
              </div>
            </div>

            {/* Form Options */}
            <div className="flex items-center justify-between">
              <label className="flex items-center gap-2 cursor-pointer group">
                <div className="relative">
                  <input
                    type="checkbox"
                    checked={keepSignedIn}
                    onChange={(e) => setKeepSignedIn(e.target.checked)}
                    className="peer sr-only"
                  />
                  <div className="w-5 h-5 border-2 border-slate-300 rounded peer-checked:bg-slate-900 peer-checked:border-slate-900 transition-all flex items-center justify-center">
                    {keepSignedIn && (
                      <svg className="w-3 h-3 text-white" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                        <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={3} d="M5 13l4 4L19 7" />
                      </svg>
                    )}
                  </div>
                </div>
                <span className="text-sm text-slate-700 group-hover:text-slate-900">Keep me signed in</span>
              </label>

              <Link 
                href="/forgot-password" 
                className="text-sm text-slate-600 hover:text-slate-900 transition-colors"
              >
                Forgot Password?
              </Link>
            </div>

            {/* Submit Button */}
            <button
              type="submit"
              className="w-full bg-slate-900 text-white py-3 rounded-lg font-semibold hover:bg-slate-800 transition-colors shadow-lg hover:shadow-xl"
            >
              Continue
            </button>

            {/* Sign Up Link */}
            <p className="text-center text-slate-600">
              Don't have an Account?{' '}
              <Link href="/signup" className="text-slate-900 font-semibold hover:underline">
                Sign up here
              </Link>
            </p>
          </form>
        </div>
      </div>

      {/* Q&R Bubble */}
      <div className="fixed bottom-8 right-8 w-16 h-16 bg-gradient-to-br from-slate-800 to-slate-900 rounded-full flex items-center justify-center text-white font-bold shadow-lg hover:shadow-xl transition-all duration-300 hover:scale-110 cursor-pointer z-50">
        <span className="text-sm">Q&R</span>
      </div>
    </div>
  );
}