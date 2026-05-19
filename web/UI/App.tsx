import { FluentProvider, webLightTheme } from '@fluentui/react-components';
import { Provider } from 'jotai';
import { ReactElement } from 'react';
import { KeepAlive } from '../KeepAlive';
import { getStore } from '../State/Storage';
import { Sidebar } from './Sidebar';
import { ViewSelector } from './Views/Selector';

import '../styles/App.css';

function TheActualApp(): ReactElement {
  return (
    <div id="app">
      The App Lives!
      <span id="left-column">left</span>
      <span id="top-row">top</span>
      <Sidebar />
      <ViewSelector />
    </div>
  );
}

export function App(): ReactElement {
  const store = getStore();
  return (
    <Provider store={store}>
      <FluentProvider theme={webLightTheme}>
        <KeepAlive />
        <TheActualApp />
      </FluentProvider>
    </Provider>
  );
}
