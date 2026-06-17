/**
 * This file is the entry point for the React app, it sets up the root
 * element and renders the App component to the DOM.
 *
 * It is included in `src/index.html`.
 */

import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { Ready, WebSocketRegistrar } from './Tools/ReactWebSocket';
import { App } from './UI/App';

const elem = document.getElementById('root')!;

const app = (
  <StrictMode>
    <App />
  </StrictMode>
);

if (false /* import.meta.hot */) {
  // With hot module reloading, `import.meta.hot.data` is persisted.
  //.. const root = (import.meta.hot.data.root ??= createRoot(elem));
  //..  root.render(app);
  // console.log(theActualApp);
  // TODO: Make HMR work with Crow
} else {
  WebSocketRegistrar();
  // The hot module reloading API is not available in production.
  createRoot(elem).render(app);
}

// This is the thing to tell the server to quit when the page is closed
// window.addEventListener('beforeunload', () => {}); void RawGet('/quit'));

window.addEventListener('DOMContentLoaded', () => {
  Ready('content');
});
