import { DefaultButton } from '@fluentui/react';
import { Expandable, StateToggle } from '@freik/fluentui-tools';
import {
  isArrayOfString,
  isBoolean,
  isString,
  isUndefined,
} from '@freik/typechk';
import { CSSProperties, ReactElement, useState } from 'react';
import {
  IpcCall,
  OpenDialogOptions,
  StorageId,
} from '../../Shared/CommonTypes';
import { useJotaiBoolState } from '../../State/Hooks';
import { atomWithMainStorage } from '../../State/Storage';
import { PostMain } from '../../Tools/Ipc';
import { ShowOpenDialog } from '../../Tools/Utilities';

import './styles/Settings.css';

const btnWidth: CSSProperties = { width: '155px', padding: 0 };

const setting = atomWithMainStorage(StorageId.SettingValue, true, isBoolean);

function ArticleSorting(): ReactElement {
  const articles = useJotaiBoolState(setting);
  return <StateToggle label="Ignore articles when sorting" state={articles} />;
}

export function SettingsView(): ReactElement {
  const [Data, setData] = useState<string>('<uninitialized>');
  return (
    <div className="settings-view">
      <Expandable separator label="Some Things" defaultShow={true}>
        <Expandable
          indent={30}
          separator
          label="Ignore filters"
          defaultShow={false}>
          <div>Something here</div>
        </Expandable>
      </Expandable>
      <Expandable separator label="Sorting & Filtering" defaultShow={true}>
        <ArticleSorting />
      </Expandable>
      <Expandable separator label="Some Things" defaultShow={true}>
        <>
          <DefaultButton
            text="Call a Thing"
            style={btnWidth}
            onClick={() => PostMain(IpcCall.RestoreWindow)}
          />
          &nbsp;
          <DefaultButton
            text="Clear another Thing"
            style={btnWidth}
            onClick={() => PostMain(IpcCall.MaximizeWindow)}
          />
          <DefaultButton
            text="Show Open File Dialog"
            style={btnWidth}
            onClick={() => {
              const odo: OpenDialogOptions = {
                folder: true,
                title: 'This is the title',
              };
              ShowOpenDialog(odo).then((val) => {
                if (isUndefined(val)) {
                  setData('Undefined result');
                } else if (isArrayOfString(val)) {
                  setData(val.join('; '));
                } else if (isString(val)) {
                  setData(val);
                } else {
                  setData(
                    'non-string result from OFD:' +
                      JSON.stringify(val).toString(),
                  );
                }
              });
            }}
          />
          <div>{Data}</div>
        </>
      </Expandable>
    </div>
  );
}
