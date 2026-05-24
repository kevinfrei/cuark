import { Expandable } from '@freik/fluentui-tools';
import { ReactElement, Suspense } from 'react';

import { ErrorBoundary } from '../../Tools/Utilities';
import { FolderChooser } from '../Dialogs/FolderChooser';
import './styles/Tools.css';

export function ToolsView(): ReactElement {
  return (
    <div className="tools-view">
      <Expandable separator label="Some Tool" defaultShow>
        <div>Put some tool thing in here.</div>
      </Expandable>
      <Expandable separator label="Hidden Tool">
        <div>Put some less commonly used tool thing in here.</div>
      </Expandable>
      <ErrorBoundary>
        <Suspense fallback={<p>Loading file system shit...</p>}>
          <FolderChooser />
        </Suspense>
      </ErrorBoundary>
    </div>
  );
}
