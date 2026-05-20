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

import './styles/folder-chooser.css';

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
        <div className="chooser-container">
          <div className="chooser-places">
            <div>Locations</div>
            {locations.map((loc) => (
              <div key={loc}>{loc}</div>
            ))}
            <div>
              <div>Favorites</div>
              {favorites.map((fav) => (
                <div key={fav}>{fav}</div>
              ))}
            </div>
          </div>
          <Button className="chooser-back">&lt;</Button>
          <Button className="chooser-fwd">&gt;</Button>
          <Button className="chooser-opt">Opts</Button>
          <div className="chooser-cur">{curFolder}</div>
          <Button className="chooser-view">View</Button>
          <div className="chooser-content">Folder List Here</div>
          <div className="chooser-actions">
            <Button className="chooser-newfolder" appearance="secondary">
              New Folder
            </Button>
            <Button className="chooser-cancel" appearance="secondary">
              Cancel
            </Button>
            <DialogTrigger disableButtonEnhancement>
              <Button className="chooser-select">Select</Button>
            </DialogTrigger>
          </div>
        </div>
      </DialogSurface>
    </Dialog>
  );
}
