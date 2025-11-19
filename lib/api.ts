// lib/api.ts - Centralized API service layer

const API_BASE_URL = process.env.NEXT_PUBLIC_API_BASE_URL || 'http://localhost:8080/api';

class ApiService {
  private baseUrl: string;

  constructor(baseUrl: string) {
    this.baseUrl = baseUrl;
  }

  private async request<T>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<T> {
    const url = `${this.baseUrl}${endpoint}`;
    
    const config: RequestInit = {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...options.headers,
      },
    };

    try {
      const response = await fetch(url, config);
      
      if (!response.ok) {
        throw new Error(`API Error: ${response.status} ${response.statusText}`);
      }

      return await response.json();
    } catch (error) {
      console.error('API Request failed:', error);
      throw error;
    }
  }

  // Crypto endpoints
  async getCryptoList() {
    return this.request('/crypto/list');
  }

  async getCryptoPrice(symbol: string) {
    return this.request(`/crypto/price/${symbol}`);
  }

  async getHistoricalData(symbol: string, interval = '1h', limit = 100) {
    return this.request(`/crypto/history/${symbol}?interval=${interval}&limit=${limit}`);
  }

  // AI endpoints
  async sendChatMessage(message: string) {
    return this.request('/ai/chat', {
      method: 'POST',
      body: JSON.stringify({ message }),
    });
  }

  async getPrediction(symbol: string, data: any) {
    return this.request(`/ai/predict/${symbol}`, {
      method: 'POST',
      body: JSON.stringify({ data }),
    });
  }

  // Authentication endpoints (to be implemented in backend)
  async login(email: string, password: string) {
    return this.request('/auth/login', {
      method: 'POST',
      body: JSON.stringify({ email, password }),
    });
  }

  async signup(email: string, password: string) {
    return this.request('/auth/signup', {
      method: 'POST',
      body: JSON.stringify({ email, password }),
    });
  }

  async logout() {
    return this.request('/auth/logout', {
      method: 'POST',
    });
  }
}

export const apiService = new ApiService(API_BASE_URL);

// Type definitions
export interface CryptoData {
  symbol: string;
  price: string;
  priceChange: string;
  priceChangePercent: string;
  volume: string;
  high24h?: string;
  low24h?: string;
}

export interface HistoricalData {
  time: number;
  open: string;
  high: string;
  low: string;
  close: string;
  volume: string;
}

export interface ChatResponse {
  candidates: Array<{
    content: {
      parts: Array<{
        text: string;
      }>;
    };
  }>;
}

export interface PredictionResponse extends ChatResponse {}

export interface AuthResponse {
  token: string;
  user: {
    id: string;
    email: string;
    name: string;
  };
}