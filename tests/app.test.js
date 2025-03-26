const fs = require('fs');
const path = require('path');
const { JSDOM } = require('jsdom');

describe('Frontend Tests', () => {
    let dom;
    let window;
    let document;
    let fetchMock;

    beforeEach(() => {
        // Setup mock DOM
        dom = new JSDOM(
            fs.readFileSync(
                path.resolve(__dirname, '../data/www/index.html'),
                'utf8'
            ),
            { runScripts: 'dangerously' }
        );
        window = dom.window;
        document = window.document;

        // Mock fetch
        fetchMock = jest.fn(() =>
            Promise.resolve({
                ok: true,
                json: () => Promise.resolve({
                    wifi: { mode: 1, ssid: 'test', channel: 1, hidden: false },
                    message: { active: 1, count: 2 },
                    messages: [
                        { id: 1, text: 'Test 1' },
                        { id: 2, text: 'Test 2' }
                    ],
                    volume: 50
                })
            })
        );
        global.fetch = fetchMock;
    });

    test('should load configuration on init', async () => {
        // Load app.js manually since JSDOM doesn't load scripts
        const appJs = fs.readFileSync(
            path.resolve(__dirname, '../data/www/js/app.js'),
            'utf8'
        );
        const scriptEl = document.createElement('script');
        scriptEl.textContent = appJs;
        document.body.appendChild(scriptEl);

        // Wait for DOM content loaded
        await new Promise(resolve => setTimeout(resolve, 100));

        expect(fetchMock).toHaveBeenCalledWith('/api/config', expect.any(Object));
    });

    test('should update message list correctly', () => {
        window.config = {
            messages: [
                { id: 1, text: 'Test 1' },
                { id: 2, text: 'Test 2' }
            ],
            message: { active: 1 },
            volume: 50
        };
        window.updateMessageTab();
        
        const messageList = document.getElementById('messageList');
        expect(messageList.querySelectorAll('.msg-container').length).toBe(2);
    });
});
