import { Expandable } from '@freik/fluentui-tools';
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
import { atomWithMainStorage } from '../../State/Storage';
import { PostMain } from '../../Tools/Ipc';
import { ShowOpenDialog } from '../../Tools/Utilities';

import { Button, Switch } from '@fluentui/react-components';
import { useAtom } from 'jotai';
import './styles/Settings.css';

const btnWidth: CSSProperties = { width: '155px', padding: 0 };

const setting = atomWithMainStorage(StorageId.SettingValue, true, isBoolean);

function ArticleSorting(): ReactElement {
  const [articles, setArticles] = useAtom(setting);
  return (
    <Switch
      label="Ignore articles when sorting"
      checked={articles}
      onChange={() => setArticles(!articles)}
    />
  );
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
          <Button
            appearance="primary"
            style={btnWidth}
            onClick={() => PostMain(IpcCall.RestoreWindow)}>
            Call a Thing
          </Button>
          &nbsp;
          <Button
            appearance="secondary"
            style={btnWidth}
            onClick={() => PostMain(IpcCall.MaximizeWindow)}>
            Clear another Thing
          </Button>
          <Button
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
            }}>
            Show Open File Dialog
          </Button>
          <div>{Data}</div>
        </>
      </Expandable>
    </div>
  );
}
