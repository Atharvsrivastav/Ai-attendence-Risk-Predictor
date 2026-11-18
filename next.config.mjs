/** @type {import('next').NextConfig} */
const backendUrl = process.env.BACKEND_URL || 'http://127.0.0.1:8080';

if (process.env.NODE_ENV === 'production' && !process.env.BACKEND_URL) {
  console.warn(
    '⚠️ [Production Notice] BACKEND_URL is not set. Defaulting to http://127.0.0.1:8080. ' +
    'If the C++ backend is hosted on a separate service, configure BACKEND_URL in your environment.'
  );
}

const nextConfig = {
  reactStrictMode: true,
  async rewrites() {
    return {
      beforeFiles: [
        {
          source: '/api/:path*',
          destination: `${backendUrl.replace(/\/$/, '')}/api/:path*`,
        },
      ],
    };
  },
};

export default nextConfig;
