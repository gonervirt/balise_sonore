const axios = require('axios');

const API_URL = 'http://localhost:5000/api';

describe('Balise Sonore API', () => {
  test('GET /status should return ok', async () => {
    const response = await axios.get(`${API_URL}/status`);
    expect(response.data).toEqual({ status: 'ok' });
  });

  test('GET /config should return configuration', async () => {
    const response = await axios.get(`${API_URL}/config`);
    expect(response.data).toHaveProperty('wifi');
    expect(response.data).toHaveProperty('message');
    expect(response.data).toHaveProperty('messages');
    expect(response.data).toHaveProperty('volume');
  });
});
