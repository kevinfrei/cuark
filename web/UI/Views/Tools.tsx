import { Expandable } from '@freik/fluentui-tools';
import { ReactElement } from 'react';

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
    </div>
  );
}
