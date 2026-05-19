import { DialogContent } from '@fluentui/react';
import {
  Button,
  Dialog,
  DialogActions,
  DialogBody,
  DialogSurface,
  DialogTitle,
  DialogTrigger,
} from '@fluentui/react-components';
import { ReactElement, useState } from 'react';

export function FolderChooser(props: {}): ReactElement {
  const locations = ['C:\\', 'D:\\'];
  const favorites = ['Home', 'Downloads', 'Music', 'Pictures', 'Videos'];
  const [curFolder, setCurFolder] = useState<string>('C:\\Users\\freik');
  return (
    <Dialog>
      <DialogTrigger>
        <Button>Open Folder Chooser</Button>
      </DialogTrigger>
      <DialogSurface>
        <DialogBody>
          <DialogTitle>Select a folder</DialogTitle>
          <DialogContent>
            <span>
              <div>
                <div>Locations</div>
                {locations.map((loc) => (
                  <div key={loc}>{loc}</div>
                ))}
              </div>
              <div>
                <div>Favorites</div>
                {favorites.map((fav) => (
                  <div key={fav}>{fav}</div>
                ))}
              </div>
            </span>
            <span>
              <div>
                <Button>Back</Button>
                <Button>Fwd</Button>
                <Button>Dlg Options</Button>
                <Button>View Options</Button>
                <span>{curFolder}</span>
                <span>Search?</span>
              </div>
              <div>Folder List Here</div>
            </span>
          </DialogContent>
          <DialogActions>
            <Button appearance="secondary">New Folder</Button>
            <Button appearance="secondary">Cancel</Button>
            <DialogTrigger disableButtonEnhancement>
              <Button>Select</Button>
            </DialogTrigger>
          </DialogActions>
        </DialogBody>
      </DialogSurface>
    </Dialog>
  );
}
