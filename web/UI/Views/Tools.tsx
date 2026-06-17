import { Expandable } from '@freik/fluent9-tools';
import { ReactElement, Suspense, useState } from 'react';
import { ErrorBoundary } from '../../Tools/Utilities';
import { FolderChooser } from '../Dialogs/FolderChooser';
import './styles/Tools.css';

export function ToolsView(): ReactElement {
  const [selectedFolder, setSelectedFolder] = useState<string | null>(null);
  return (
    <div className="tools-view">
      <Expandable label="Some Tool" defaultShow>
        <div>Put some tool thing in here.</div>
      </Expandable>
      <Expandable separator label="Hidden Tool">
        <div>Put some less commonly used tool thing in here.</div>
      </Expandable>
      <ErrorBoundary>
        <Suspense fallback={<p>Loading file system shit...</p>}>
          <FolderChooser setResult={setSelectedFolder} text="Choose Wisely!" />
        </Suspense>
      </ErrorBoundary>
      <div>
        <span>Selected folder:&nbsp;</span>
        <span>{selectedFolder === null ? "nuthin'" : selectedFolder}</span>
      </div>
    </div>
  );
}
