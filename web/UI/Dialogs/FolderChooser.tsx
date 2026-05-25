import {
  Button,
  createTableColumn,
  DataGrid,
  DataGridBody,
  DataGridCell,
  DataGridHeader,
  DataGridHeaderCell,
  DataGridProps,
  DataGridRow,
  Dialog,
  DialogBody,
  DialogSurface,
  DialogTrigger,
  makeStyles,
  Menu,
  MenuButton,
  MenuDivider,
  MenuItemCheckbox,
  MenuItemRadio,
  MenuList,
  MenuPopover,
  MenuTrigger,
  TableColumnDefinition,
} from '@fluentui/react-components';
import {
  ChevronDown20Filled,
  ChevronLeft20Filled,
  ChevronRight20Filled,
  OptionsRegular,
} from '@fluentui/react-icons';
import { isArrayOfString } from '@freik/typechk';
import { atom, useAtom, useAtomValue, useSetAtom } from 'jotai';
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
import { CallMainThrow } from '../../Tools/Ipc';

const currentLocation = atom('');

const useStyles = makeStyles({
  surface: {
    height: '90vh',
    minWidth: '90vw',
  },
  body: {
    height: '100%',
    display: 'grid',
    gridTemplateRows: '30px auto 30px',
    gridTemplateColumns: '120px 25px 25px 30px auto',
  },
  places: {
    gridRow: '1 / span 3',
    gridColumn: 1,
    overflow: 'clip',
  },
  content: {
    gridRow: '2',
    gridColumn: '2 / span 4',
    overflow: 'auto',
  },
  back: {
    gridRow: '1',
    gridColumn: '2',
  },
  fwd: {
    gridRow: '1',
    gridColumn: '3',
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
    margin: '5px',
  },
  actions: {
    display: 'grid',
    gridRow: '3',
    gridColumn: '2 / span 4',
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

function LocationsList(): ReactElement {
  const locations = use(GetRootLocations());
  const [curLoc, setCurLoc] = useAtom(currentLocation);
  return (
    <>
      {locations.map((loc) => {
        if (curLoc === loc) {
          return (
            <div key={loc} style={{ fontWeight: 'bold' }}>
              {loc}
            </div>
          );
        } else {
          return (
            <div key={loc} onClick={() => setCurLoc(loc)}>
              {loc}
            </div>
          );
        }
      })}
    </>
  );
}

function FavoritesList(): ReactElement {
  const favorites = use(GetFavorites());
  const [curLoc, setCurLoc] = useAtom(currentLocation);
  return (
    <>
      {[...favorites.entries()].map(([key, val]) => {
        if (curLoc === val) {
          return (
            <div key={key} style={{ fontWeight: 'bold' }}>
              {key}
            </div>
          );
        } else {
          return (
            <div key={key} onClick={() => setCurLoc(val)}>
              {key}
            </div>
          );
        }
      })}
    </>
  );
}

type FilePlacesProps = {
  className: string;
};

function FilePlaces({ className }: FilePlacesProps): ReactElement {
  return (
    <div className={className}>
      <div
        key={'locations-header'}
        style={{ fontWeight: 'bold', fontSize: 'larger' }}>
        Locations
      </div>
      <Suspense>
        <LocationsList />
      </Suspense>
      <div
        key="favorites-header"
        style={{ fontWeight: 'bold', fontSize: 'larger', marginTop: '1em' }}>
        Favorites
      </div>
      <Suspense>
        <FavoritesList />
      </Suspense>
    </div>
  );
}

function Location(): ReactElement {
  const classes = useStyles();
  const curLocVal = useAtomValue(currentLocation);
  return <div className={classes.cur}>{curLocVal}</div>;
}

function FileFolderPickerHeader(): ReactElement {
  const classes = useStyles();
  return (
    <>
      <Button className={classes.back} icon={<ChevronLeft20Filled />} />
      <Button className={classes.fwd} icon={<ChevronRight20Filled />} />
      <Menu>
        <MenuTrigger>
          <MenuButton
            className={classes.opt}
            icon={
              <>
                <OptionsRegular />
                <ChevronDown20Filled />
              </>
            }
            size="large"
          />
        </MenuTrigger>
        <MenuPopover>
          <MenuList>
            <MenuItemCheckbox name="sidebar" value="true">
              Show Sidebar
            </MenuItemCheckbox>
            <MenuDivider />
            <MenuItemCheckbox name="hidden" value="false">
              Show hidden files
            </MenuItemCheckbox>
            <MenuItemCheckbox name="extensions" value="true">
              Show file extensions
            </MenuItemCheckbox>
            <MenuDivider />
            <MenuItemRadio name="view" value="icons" persistOnClick={true}>
              View as Icons
            </MenuItemRadio>
            <MenuItemRadio name="view" value="list" persistOnClick={true}>
              View as List
            </MenuItemRadio>
          </MenuList>
        </MenuPopover>
      </Menu>
      <Location />
    </>
  );
}

function FileFolderPickerFooter(): ReactElement {
  const classes = useStyles();
  return (
    <div className={classes.actions}>
      <Button className={classes.new} appearance="secondary">
        New Folder
      </Button>
      <Button className={classes.cancel} appearance="secondary">
        Cancel
      </Button>
      <DialogTrigger disableButtonEnhancement>
        <Button className={classes.select}>Select</Button>
      </DialogTrigger>
    </div>
  );
}

const columns: TableColumnDefinition<FileSystemItem>[] = [
  createTableColumn<FileSystemItem>({
    columnId: 'file',
    compare: (a, b) => a.file.localeCompare(b.file),
    renderHeaderCell: () => 'File Name',
    renderCell: (item) => item.file,
  }),
  createTableColumn<FileSystemItem>({
    columnId: 'size',
    compare: (a, b) => (a.size < b.size ? -1 : a.size > b.size ? 1 : 0),
    renderHeaderCell: () => 'File Size',
    renderCell: (item) => (item.type === 'directory' ? '' : item.size),
  }),
  createTableColumn<FileSystemItem>({
    columnId: 'date',
    compare: (a, b) => a.date - b.date,
    renderHeaderCell: () => 'Date',
    // item.date is millisecondcs since epoch, so we can create a Date object from it and format it as needed
    renderCell: (item) => new Date(item.date).toLocaleString(),
  }),
  createTableColumn<FileSystemItem>({
    columnId: 'type',
    compare: (a, b) => a.type.localeCompare(b.type),
    renderHeaderCell: (data) => 'Type',
    renderCell: (item) => item.type,
  }),
];

function FileFolderPickerContent(): ReactElement {
  const curLocVal = useAtomValue(currentLocation);
  const data = use(GetFolderContents(curLocVal));
  const classes = useStyles();
  const defaultSortState = useMemo<
    Parameters<NonNullable<DataGridProps['onSortChange']>>[1]
  >(() => ({ sortColumn: 'file', sortDirection: 'ascending' }), []);

  const columnSizingOptions = {
    file: { minWidth: 80, defaultWidth: 120 },
    size: { minWidth: 10, defaultWidth: 60 },
    date: { minWidth: 20, defaultWidth: 80 },
    type: { minWidth: 20, defaultWidth: 60 },
  };

  return (
    <DataGrid
      className={classes.content}
      items={data}
      columns={columns}
      sortable
      defaultSortState={defaultSortState}
      style={{ minWidth: '500px' }}
      size="small"
      resizableColumns
      resizableColumnsOptions={{ autoFitColumns: true }}
      columnSizingOptions={columnSizingOptions}>
      <DataGridHeader>
        <DataGridRow>
          {({ renderHeaderCell }) => (
            <DataGridHeaderCell>{renderHeaderCell()}</DataGridHeaderCell>
          )}
        </DataGridRow>
      </DataGridHeader>
      <DataGridBody<FileSystemItem>>
        {({ item, rowId }) => (
          <DataGridRow<FileSystemItem> key={rowId}>
            {({ renderCell }) => (
              <DataGridCell>{renderCell(item)}</DataGridCell>
            )}
          </DataGridRow>
        )}
      </DataGridBody>
    </DataGrid>
  );
}

type FileFolderPickerProps = {
  onChangeFolder: (path: string) => void;
};

function FileFolderPicker({
  onChangeFolder,
}: FileFolderPickerProps): ReactElement {
  return (
    <>
      <FileFolderPickerHeader />
      <Suspense>
        <FileFolderPickerContent />
      </Suspense>
      <FileFolderPickerFooter />
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
  const classes = useStyles();
  const setCurLoc = useSetAtom(currentLocation);
  return (
    <>
      <Suspense>
        <FilePlaces className={classes.places} />
      </Suspense>
      <Suspense>
        <FileFolderPicker onChangeFolder={setCurLoc} />
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
