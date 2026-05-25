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
import {
  ReactElement,
  Suspense,
  use,
  useEffect,
  useMemo,
  useState,
} from 'react';
import {
  chkFolderContents,
  chkNamedLocations,
  FileSystemItem,
  IpcCall,
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

let cachedNL: NamedLocations | null = null;
async function GetFavorites(skipCache?: boolean): Promise<NamedLocations> {
  while (cachedNL === null || skipCache) {
    cachedNL = await CallMainThrow(
      IpcCall.GetNamedLocations,
      chkNamedLocations,
    );
    skipCache = false;
  }
  return cachedNL;
}

let cachedRoots: null | string[] = null;
async function GetRootLocations(skipCache?: boolean): Promise<string[]> {
  while (cachedRoots === null || skipCache) {
    cachedRoots = await CallMainThrow(
      IpcCall.GetFileSystemRoots,
      isArrayOfString,
    );
    skipCache = false;
  }
  return cachedRoots;
}

const cachedFC = new Map<string, FileSystemItem[]>();
async function GetFolderContents(
  filePath: string,
  skipCache?: boolean,
): Promise<FileSystemItem[]> {
  if (filePath.length === 0) {
    return [];
  }
  let val: undefined | FileSystemItem[] = undefined;
  do {
    val = cachedFC.get(filePath);
    if (!val || skipCache) {
      cachedFC.set(
        filePath,
        await CallMainThrow(
          IpcCall.GetFolderContents,
          chkFolderContents,
          filePath,
        ),
      );
    }
  } while (val === undefined);
  return val;
}

const columns = [
  { colKey: 'filename', label: 'File Name' },
  { colKey: 'size', label: 'Size' },
  { colKey: 'date', label: 'Date Modified' },
  { colKey: 'kind', label: 'Type' },
];

type FilePlacesProps = {
  onSelect: (loc: string) => void;
  className: string;
};

function FilePlaces({ onSelect, className }: FilePlacesProps): ReactElement {
  const locations = use(GetRootLocations());
  const favorites = use(GetFavorites());
  return (
    <div className={className}>
      <div key={'locations-header'} style={{ fontWeight: 'bold' }}>
        Locations
      </div>
      {locations.map((loc) => (
        <div key={loc} onClick={() => onSelect(loc)}>
          {loc}
        </div>
      ))}
      <div key="favorites=header" style={{ fontWeight: 'bold' }}>
        Favorites
      </div>
      {[...favorites.entries()].map(([key, val]) => (
        <div key={key} onClick={() => onSelect(val)}>
          {key}
        </div>
      ))}
    </div>
  );
}

type FileFolderPickerProps = {
  location: string;
  onChangeFolder: (path: string) => void;
};

function FileFolderPicker({
  location,
  onChangeFolder,
}: FileFolderPickerProps): ReactElement {
  const data = use(GetFolderContents(location));
  const classes = useStyles();
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
    <>
      <Button className={backClassName} icon={<BackIcon />} />
      <Button className={fwdClassName} icon={<ForwardIcon />} />
      <Button className={optClassName} icon={<OptionsRegular />} />
      <div className={curClassName}>{location}</div>
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
    </>
  );
}

const DelayedFallback = ({
  delay,
  children,
}: {
  delay: number;
  children: ReactElement;
}) => {
  const [show, setShow] = useState(false);

  useEffect(() => {
    const timer = setTimeout(() => setShow(true), delay);
    return () => clearTimeout(timer);
  }, [delay]);

  return show ? children : null;
};

function FileFolderSurface(): ReactElement {
  const [curFolder, setCurFolder] = useState<string>('');
  const classes = useStyles();
  return (
    <>
      <Suspense>
        <FilePlaces className={classes.places} onSelect={setCurFolder} />
      </Suspense>
      <Suspense>
        <FileFolderPicker location={curFolder} onChangeFolder={setCurFolder} />
      </Suspense>
    </>
  );
}

export function FolderChooser(props: {}): ReactElement {
  const classes = useStyles();
  return (
    <Dialog>
      <DialogTrigger>
        <Button>Open Folder Chooser</Button>
      </DialogTrigger>
      <DialogSurface className={classes.surface}>
        <DialogBody className={classes.body}>
          <FileFolderSurface />
        </DialogBody>
      </DialogSurface>
    </Dialog>
  );
}
