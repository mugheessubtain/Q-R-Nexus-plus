'use client';

import { useState, useEffect } from 'react';
import { 
  Menu, RefreshCw, User, X, Send, TrendingUp, 
  TrendingDown, BarChart3, MessageSquare 
} from 'lucide-react';
import { 
  LineChart, Line, BarChart, Bar, XAxis, YAxis, 
  CartesianGrid, Tooltip, ResponsiveContainer 
} from 'recharts';

// Types
interface CryptoData {
  symbol: string;
  price: string;
  priceChange: string;
  priceChangePercent: string;
  volume: string;
}

interface OrderHistory {
  symbol: string;
  type: 'LIMIT' | 'BUY';
  filledAmount: string;
  price: string;
}

interface ChartData {
  time: string;
  price: number;
  volume: number;
}

interface ChatMessage {
  role: 'user' | 'assistant';
  content: string;
}

export default function TradingDashboard() {
  // State management
  const [selectedCrypto, setSelectedCrypto] = useState('BTC/USDT');
  const [cryptoList, setCryptoList] = useState<CryptoData[]>([]);
  const [chartData, setChartData] = useState<ChartData[]>([]);
  const [orderHistory, setOrderHistory] = useState<OrderHistory[]>([]);
  const [availableFunds, setAvailableFunds] = useState(100456.328);
  const [chatMessages, setChatMessages] = useState<ChatMessage[]>([]);
  const [chatInput, setChatInput] = useState('');
  const [prediction, setPrediction] = useState<any>(null);
  const [loading, setLoading] = useState(false);
  const [showChat, setShowChat] = useState(false);
  
  // Trading form state
  const [orderType, setOrderType] = useState<'Spot' | 'Isolated' | 'Margin'>('Spot');
  const [tradeType, setTradeType] = useState<'Limit' | 'Market' | 'Stop Limit'>('Limit');
  const [amount, setAmount] = useState('');
  const [price, setPrice] = useState('');
  const [leverage, setLeverage] = useState('2-60x');

  // API Base URL (Update with your backend URL)
  const API_BASE = 'http://localhost:8080/api';

  // Fetch crypto list
  useEffect(() => {
    fetchCryptoList();
    const interval = setInterval(fetchCryptoList, 30000); // Update every 30s
    return () => clearInterval(interval);
  }, []);

  // Fetch chart data when crypto changes
  useEffect(() => {
    if (selectedCrypto) {
      fetchChartData(selectedCrypto);
    }
  }, [selectedCrypto]);

  const fetchCryptoList = async () => {
    try {
      const response = await fetch(`${API_BASE}/crypto/list`);
      const data = await response.json();
      // Filter for major cryptos
      const filtered = data.filter((item: any) => 
        ['BTCUSDT', 'ETHUSDT', 'BNBUSDT', 'SOLUSDT'].includes(item.symbol)
      ).map((item: any) => ({
        symbol: item.symbol.replace('USDT', '/USDT'),
        price: parseFloat(item.lastPrice).toFixed(2),
        priceChange: item.priceChange,
        priceChangePercent: item.priceChangePercent,
        volume: item.volume
      }));
      setCryptoList(filtered);
    } catch (error) {
      console.error('Error fetching crypto list:', error);
    }
  };

  const fetchChartData = async (symbol: string) => {
    try {
      const cleanSymbol = symbol.replace('/', '');
      const response = await fetch(
        `${API_BASE}/crypto/history/${cleanSymbol}?interval=1h&limit=50`
      );
      const data = await response.json();
      
      const formatted = data.map((item: any) => ({
        time: new Date(item[0]).toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit' }),
        price: parseFloat(item[4]),
        volume: parseFloat(item[5])
      }));
      
      setChartData(formatted);
    } catch (error) {
      console.error('Error fetching chart data:', error);
    }
  };

  const fetchPrediction = async () => {
    setLoading(true);
    try {
      const cleanSymbol = selectedCrypto.replace('/', '');
      const response = await fetch(`${API_BASE}/ai/predict/${cleanSymbol}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ 
          data: JSON.stringify(chartData) 
        })
      });
      const data = await response.json();
      
      // Parse Gemini response
      const content = data.candidates[0].content.parts[0].text;
      setPrediction({
        text: content,
        symbol: selectedCrypto
      });
    } catch (error) {
      console.error('Error fetching prediction:', error);
    }
    setLoading(false);
  };
const sendChatMessage = async () => {
  if (!chatInput.trim()) return;

  const userMessage: ChatMessage = { role: 'user', content: chatInput };
  setChatMessages(prev => [...prev, userMessage]);
  setLoading(true);

  try {
    const response = await fetch('/api/chat', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ message: chatInput })
    });

    const data = await response.json();

    // Adjust based on Gemini's response structure
    const assistantMessage: ChatMessage = {
  role: 'assistant',
  content: data.reply || 'No response'
};

    setChatMessages(prev => [...prev, assistantMessage]);
  } catch (error) {
    console.error('Error sending chat message:', error);
  }

  setLoading(false);
};

  // const sendChatMessage = async () => {
  //   if (!chatInput.trim()) return;

  //   const userMessage: ChatMessage = { role: 'user', content: chatInput };
  //   setChatMessages(prev => [...prev, userMessage]);
  //   setChatInput('');
  //   setLoading(true);

  //   try {
  //     const response = await fetch(`${API_BASE}/chat`, {
  //       method: 'POST',
  //       headers: { 'Content-Type': 'application/json' },
  //       body: JSON.stringify({ message: chatInput })
  //     });
  //     const data = await response.json();
      
  //     const assistantMessage: ChatMessage = {
  //       role: 'assistant',
  //       content: data.candidates[0].content.parts[0].text
  //     };
  //     setChatMessages(prev => [...prev, assistantMessage]);
  //   } catch (error) {
  //     console.error('Error sending chat message:', error);
  //   }
  //   setLoading(false);
  // };

  return (
    <div className="min-h-screen bg-neutral-900 text-white">
      {/* Header */}
      <header className="bg-neutral-800 border-b border-neutral-700 px-6 py-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-4">
            <button className="p-2 hover:bg-neutral-700 rounded-lg transition-colors">
              <Menu className="w-6 h-6" />
            </button>
            <div className="relative">
              <input
                type="text"
                placeholder="Search for crypto, stock or commodity..."
                className="w-96 bg-neutral-700 px-4 py-2 rounded-lg focus:outline-none focus:ring-2 focus:ring-cyan-500"
              />
            </div>
          </div>
          
          <div className="flex items-center gap-4">
            <button 
              onClick={() => setShowChat(!showChat)}
              className="flex items-center gap-2 px-4 py-2 bg-cyan-600 hover:bg-cyan-700 rounded-lg transition-colors"
            >
              <MessageSquare className="w-5 h-5" />
              AI Assistant
            </button>
            <button className="p-2 hover:bg-neutral-700 rounded-lg transition-colors">
              <RefreshCw className="w-6 h-6" />
            </button>
            <button className="flex items-center gap-2 px-4 py-2 bg-neutral-700 hover:bg-neutral-600 rounded-lg transition-colors">
              <User className="w-5 h-5" />
              Abdullah Khan
            </button>
          </div>
        </div>

        {/* Crypto Pills */}
        <div className="flex gap-2 mt-4">
          {cryptoList.slice(0, 2).map((crypto) => (
            <button
              key={crypto.symbol}
              onClick={() => setSelectedCrypto(crypto.symbol)}
              className={`px-4 py-2 rounded-lg flex items-center gap-2 transition-colors ${
                selectedCrypto === crypto.symbol
                  ? 'bg-neutral-700 border border-cyan-500'
                  : 'bg-neutral-700 hover:bg-neutral-600'
              }`}
            >
              {crypto.symbol}
              <X className="w-4 h-4 text-neutral-400" />
            </button>
          ))}
        </div>
      </header>

      <div className="flex">
        {/* Left Sidebar */}
        <aside className="w-80 bg-neutral-800 border-r border-neutral-700 p-4 h-[calc(100vh-140px)] overflow-y-auto">
          {/* Trading Type Tabs */}
          <div className="flex gap-2 mb-4">
            {(['Spot', 'Isolated', 'Margin'] as const).map((type) => (
              <button
                key={type}
                onClick={() => setOrderType(type)}
                className={`flex-1 py-2 rounded-lg text-sm font-medium transition-colors ${
                  orderType === type
                    ? 'bg-cyan-600 text-white'
                    : 'bg-neutral-700 text-neutral-300 hover:bg-neutral-600'
                }`}
              >
                {type}
              </button>
            ))}
          </div>

          {/* Order Type Tabs */}
          <div className="flex gap-2 mb-4">
            {(['Limit', 'Market', 'Stop Limit'] as const).map((type) => (
              <button
                key={type}
                onClick={() => setTradeType(type)}
                className={`flex-1 py-2 rounded-lg text-xs transition-colors ${
                  tradeType === type
                    ? 'bg-neutral-700 text-white'
                    : 'bg-neutral-900 text-neutral-400 hover:bg-neutral-700'
                }`}
              >
                {type}
              </button>
            ))}
          </div>

          {/* Trading Inputs */}
          <div className="space-y-3 mb-4">
            <input
              type="text"
              placeholder="Amount"
              value={amount}
              onChange={(e) => setAmount(e.target.value)}
              className="w-full bg-neutral-700 px-3 py-2 rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-cyan-500"
            />
            <input
              type="text"
              placeholder="Price"
              value={price}
              onChange={(e) => setPrice(e.target.value)}
              className="w-full bg-neutral-700 px-3 py-2 rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-cyan-500"
            />
            <div className="grid grid-cols-2 gap-2">
              <input
                type="text"
                placeholder="TP"
                className="bg-neutral-700 px-3 py-2 rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-cyan-500"
              />
              <input
                type="text"
                placeholder="SL"
                className="bg-neutral-700 px-3 py-2 rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-cyan-500"
              />
            </div>
            <select
              value={leverage}
              onChange={(e) => setLeverage(e.target.value)}
              className="w-full bg-neutral-700 px-3 py-2 rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-cyan-500"
            >
              <option>2-60x</option>
              <option>10x</option>
              <option>25x</option>
              <option>50x</option>
            </select>
          </div>

          <div className="mb-4 p-3 bg-neutral-700 rounded-lg">
            <div className="flex justify-between text-sm mb-2">
              <span className="text-neutral-400">Available Funds</span>
              <span className="font-semibold">{availableFunds.toFixed(3)}</span>
            </div>
          </div>

          {/* Order History */}
          <div className="mb-4">
            <h3 className="text-sm font-semibold mb-2">Order History</h3>
            <div className="space-y-2">
              {[
                { symbol: 'BTC/USDT', type: 'LIMIT/BUY', filled: '0.00/20.02', price: '100,012' },
                { symbol: 'AAPL/NASDAQ', type: 'LIMIT/BUY', filled: '0.00/20.02', price: '100,012' }
              ].map((order, idx) => (
                <div key={idx} className="bg-neutral-700 p-3 rounded-lg">
                  <div className="flex justify-between text-xs mb-1">
                    <span className="font-semibold">{order.symbol}</span>
                    <span className="text-cyan-400">{order.type}</span>
                  </div>
                  <div className="flex justify-between text-xs text-neutral-400">
                    <span>FILLED/AMOUNT: {order.filled}</span>
                    <span>PRICE: {order.price}</span>
                  </div>
                </div>
              ))}
            </div>
          </div>
        </aside>

        {/* Main Chart Area */}
        <main className="flex-1 p-6">
          <div className="bg-neutral-800 rounded-lg p-4 h-[calc(100vh-180px)]">
            <ResponsiveContainer width="100%" height="70%">
              <LineChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" stroke="#374151" />
                <XAxis dataKey="time" stroke="#9CA3AF" />
                <YAxis stroke="#9CA3AF" />
                {/* <Tooltip 
                  contentStyle={{ 
                    backgroundColor: '#1f2937', 
                    border: '1px solid #374151',
                    borderRadius: '8px'
                  }} 
                /> */}
                <Tooltip 
  contentStyle={{ backgroundColor: '#1f2937', border: '1px solid #374151' }}
  formatter={(value) => (value != null ? value.toString() : '0')}
/>

                <Line 
                  type="monotone" 
                  dataKey="price" 
                  stroke="#06b6d4" 
                  strokeWidth={2}
                  dot={false}
                />
              </LineChart>
            </ResponsiveContainer>

            <ResponsiveContainer width="100%" height="25%">
              <BarChart data={chartData}>
                <XAxis dataKey="time" stroke="#9CA3AF" hide />
                {/* <Tooltip 
                  contentStyle={{ 
                    backgroundColor: '#1f2937', 
                    border: '1px solid #374151',
                    borderRadius: '8px'
                  }} 
                /> */}
                <Tooltip 
  contentStyle={{ backgroundColor: '#1f2937', border: '1px solid #374151' }}
  formatter={(value) => (value != null ? value.toString() : '0')}
/>

                <Bar dataKey="volume" fill="#06b6d4" opacity={0.6} />
              </BarChart>
            </ResponsiveContainer>

            <div className="mt-4">
              <input
                type="text"
                placeholder="Ask me anything about the chart..."
                className="w-full bg-neutral-700 px-4 py-3 rounded-lg focus:outline-none focus:ring-2 focus:ring-cyan-500"
                onKeyPress={(e) => {
                  if (e.key === 'Enter') {
                    setChatInput((e.target as HTMLInputElement).value);
                    sendChatMessage();
                    (e.target as HTMLInputElement).value = '';
                  }
                }}
              />
            </div>
          </div>
        </main>

        {/* Right Sidebar - AI Predictions */}
        <aside className="w-80 bg-neutral-800 border-l border-neutral-700 p-4 h-[calc(100vh-140px)] overflow-y-auto">
          <div className="mb-4">
            <button
              onClick={fetchPrediction}
              disabled={loading}
              className="w-full bg-cyan-600 hover:bg-cyan-700 disabled:bg-neutral-600 text-white py-2 rounded-lg flex items-center justify-center gap-2 transition-colors"
            >
              <BarChart3 className="w-5 h-5" />
              {loading ? 'Analyzing...' : 'Get AI Prediction'}
            </button>
          </div>

          {prediction && (
            <div className="bg-neutral-700 rounded-lg p-4 mb-4">
              <h3 className="font-semibold mb-2 flex items-center gap-2">
                <TrendingUp className="w-5 h-5 text-green-400" />
                Price Prediction for {prediction.symbol}
              </h3>
              <div className="text-sm text-neutral-300 whitespace-pre-wrap">
                {prediction.text}
              </div>
            </div>
          )}

          <div className="bg-neutral-700 rounded-lg p-4">
            <h3 className="font-semibold mb-3">Market Insights</h3>
            {cryptoList.map((crypto) => (
              <div key={crypto.symbol} className="mb-3 pb-3 border-b border-neutral-600 last:border-0">
                <div className="flex justify-between items-center mb-1">
                  <span className="font-medium">{crypto.symbol}</span>
                  <span className={`flex items-center gap-1 text-sm ${
                    parseFloat(crypto.priceChangePercent) >= 0 
                      ? 'text-green-400' 
                      : 'text-red-400'
                  }`}>
                    {parseFloat(crypto.priceChangePercent) >= 0 ? (
                      <TrendingUp className="w-4 h-4" />
                    ) : (
                      <TrendingDown className="w-4 h-4" />
                    )}
                    {Math.abs(parseFloat(crypto.priceChangePercent)).toFixed(2)}%
                  </span>
                </div>
                <div className="text-2xl font-bold">${crypto.price}</div>
                <div className="text-xs text-neutral-400">Vol: {parseFloat(crypto.volume).toFixed(0)}</div>
              </div>
            ))}
          </div>
        </aside>
      </div>

      {/* Chat Modal */}
      {showChat && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
          <div className="bg-neutral-800 rounded-lg w-full max-w-2xl h-[600px] flex flex-col">
            <div className="flex items-center justify-between p-4 border-b border-neutral-700">
              <h2 className="text-xl font-bold">AI Trading Assistant</h2>
              <button
                onClick={() => setShowChat(false)}
                className="p-2 hover:bg-neutral-700 rounded-lg transition-colors"
              >
                <X className="w-5 h-5" />
              </button>
            </div>

            <div className="flex-1 overflow-y-auto p-4 space-y-4">
              {chatMessages.length === 0 ? (
                <div className="text-center text-neutral-400 mt-8">
                  <MessageSquare className="w-12 h-12 mx-auto mb-4 opacity-50" />
                  <p>Ask me anything about trading, market analysis, or predictions!</p>
                </div>
              ) : (
                chatMessages.map((msg, idx) => (
                  <div
                    key={idx}
                    className={`flex ${msg.role === 'user' ? 'justify-end' : 'justify-start'}`}
                  >
                    <div
                      className={`max-w-[80%] p-3 rounded-lg ${
                        msg.role === 'user'
                          ? 'bg-cyan-600 text-white'
                          : 'bg-neutral-700 text-neutral-100'
                      }`}
                    >
                      <p className="text-sm whitespace-pre-wrap">{msg.content}</p>
                    </div>
                  </div>
                ))
              )}
              {loading && (
                <div className="flex justify-start">
                  <div className="bg-neutral-700 p-3 rounded-lg">
                    <div className="flex gap-2">
                      <div className="w-2 h-2 bg-cyan-500 rounded-full animate-bounce" />
                      <div className="w-2 h-2 bg-cyan-500 rounded-full animate-bounce delay-100" />
                      <div className="w-2 h-2 bg-cyan-500 rounded-full animate-bounce delay-200" />
                    </div>
                  </div>
                </div>
              )}
            </div>

            <div className="p-4 border-t border-neutral-700">
              <div className="flex gap-2">
                <input
                  type="text"
                  value={chatInput}
                  onChange={(e) => setChatInput(e.target.value)}
                  onKeyPress={(e) => e.key === 'Enter' && sendChatMessage()}
                  placeholder="Type your message..."
                  className="flex-1 bg-neutral-700 px-4 py-2 rounded-lg focus:outline-none focus:ring-2 focus:ring-cyan-500"
                />
                <button
                  onClick={sendChatMessage}
                  disabled={loading || !chatInput.trim()}
                  className="px-6 py-2 bg-cyan-600 hover:bg-cyan-700 disabled:bg-neutral-600 rounded-lg transition-colors"
                >
                  <Send className="w-5 h-5" />
                </button>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}