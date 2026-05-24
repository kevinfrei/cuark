import { DialogContent } from '@fluentui/react';
import {
  Button,
  Dialog,
  DialogBody,
  DialogSurface,
  DialogTrigger,
  makeStyles,
  mergeClasses,
  Table,
  TableBody,
  TableCell,
  TableHeader,
  TableHeaderCell,
  TableRow,
} from '@fluentui/react-components';
import { OptionsRegular, SlideGrid20Filled } from '@fluentui/react-icons';
import { BackIcon, ForwardIcon } from '@fluentui/react-icons-mdl2';
import {
  chkArrayOf,
  isArrayOfString,
  isMapOfStrings,
  isUndefined,
} from '@freik/typechk';
import { ReactElement, use, useState } from 'react';
import {
  chkFolderContents,
  chkNamedLocation,
  chkNamedLocations,
  FileSystemItem,
  IpcCall,
  NamedLocation,
  NamedLocations,
} from '../../Shared/CommonTypes';
import { CallMain, CallMainThrow } from '../../Tools/Ipc';

const useStyles = makeStyles({
  surface: {
    height: '90vh',
    width: '90vw',
  },
  body: {
    height: '100%',
    display: 'grid',
    gridTemplateRows: '30px auto 30px',
    gridTemplateColumns: '120px 40px 40px 40px auto 40px',
  },
  places: {
    gridRow: '1 / span 3',
    gridColumn: 1,
  },
  content: {
    gridRow: '2',
    gridColumn: '2 / span 5',
  },
  back: {
    gridRow: '1',
    gridColumn: '2',
    fontSize: 'small',
  },
  fwd: {
    gridRow: '1',
    gridColumn: '3',
    fontSize: 'small',
  },
  opt: {
    gridRow: '1',
    gridColumn: '4',
    fontSize: 'small',
  },
  cur: {
    gridRow: '1',
    gridColumn: '5',
    fontSize: 'small',
  },
  view: {
    gridRow: '1',
    gridColumn: '6',
    fontSize: 'small',
  },
  actions: {
    display: 'grid',
    gridRow: '3',
    gridColumn: '2 / span 5',
    gridTemplateRows: 'auto',
    gridTemplateColumns: '110px auto 100px 100px',
  },
  new: {
    gridRow: '1',
    gridColumn: '1',
  },
  cancel: {
    gridRow: '1',
    gridColumn: '3',
  },
  select: {
    gridRow: '1',
    gridColumn: '4',
  },
});

async function GetFavorites(): Promise<NamedLocations> {
  return await CallMainThrow(IpcCall.GetNamedLocations, chkNamedLocations);
}

async function GetRootLocations(): Promise<string[]> {
  return await CallMainThrow(IpcCall.GetFileSystemRoots, isArrayOfString);
}

async function GetFolderContents(filePath: string): Promise<FileSystemItem[]> {
  return await CallMainThrow(
    IpcCall.GetFolderContents,
    chkFolderContents,
    filePath,
  );
}

const columns = [
  { colKey: 'filename', label: 'File Name' },
  { colKey: 'size', label: 'Size' },
  { colKey: 'date', label: 'Date Modified' },
  { colKey: 'kind', label: 'Type' },
];

export function FolderChooser(props: {}): ReactElement {
  const locations = use(GetRootLocations());
  const favorites = use(GetFavorites());
  const [curFolder, setCurFolder] = useState<string>(
    favorites.get('home') || '',
  );
  const data = use(GetFolderContents(curFolder));
  const classes = useStyles();
  const surfaceClassName = mergeClasses(classes.surface);
  const bodyClassName = mergeClasses(classes.body);
  const placesClassName = mergeClasses(classes.places);
  const backClassName = mergeClasses(classes.back);
  const fwdClassName = mergeClasses(classes.fwd);
  const optClassName = mergeClasses(classes.opt);
  const curClassName = mergeClasses(classes.cur);
  const viewClassName = mergeClasses(classes.view);
  const contentClassName = mergeClasses(classes.content);
  const actionsClassName = mergeClasses(classes.actions);
  const newClassName = mergeClasses(classes.new);
  const cancelClassName = mergeClasses(classes.cancel);
  const selectClassName = mergeClasses(classes.select);
  return (
    <Dialog>
      <DialogTrigger>
        <Button>Open Folder Chooser</Button>
      </DialogTrigger>
      <DialogSurface className={surfaceClassName}>
        <DialogBody className={bodyClassName}>
          <div className={placesClassName}>
            <h4>Locations</h4>
            {locations.map((loc) => (
              <div key={loc}>{loc}</div>
            ))}
            <div>
              <h4>Favorites</h4>
              {[...favorites.entries()].map(([key, val]) => (
                <div key={key}>
                  {key}: {val.substring(val.length - Math.min(5, val.length))}
                </div>
              ))}
            </div>
          </div>
          <Button className={backClassName} icon={<BackIcon />} />
          <Button className={fwdClassName} icon={<ForwardIcon />} />
          <Button className={optClassName} icon={<OptionsRegular />} />
          <div className={curClassName}>{curFolder}</div>
          <Button className={viewClassName} icon={<SlideGrid20Filled />} />
          <Table className={contentClassName}>
            <TableHeader>
              <TableRow>
                {columns.map(({ colKey, label }) => (
                  <TableHeaderCell key={colKey}>{label}</TableHeaderCell>
                ))}
              </TableRow>
            </TableHeader>
            <TableBody>
              {data.map((val) => (
                <TableRow>
                  <TableCell>{val.file}</TableCell>
                  <TableCell>{val.size}</TableCell>
                  <TableCell>{val.date}</TableCell>
                  <TableCell>{val.type}</TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
          <div className={actionsClassName}>
            <Button className={newClassName} appearance="secondary">
              New Folder
            </Button>
            <Button className={cancelClassName} appearance="secondary">
              Cancel
            </Button>
            <DialogTrigger disableButtonEnhancement>
              <Button className={selectClassName}>Select</Button>
            </DialogTrigger>
          </div>
        </DialogBody>
      </DialogSurface>
    </Dialog>
  );
}
