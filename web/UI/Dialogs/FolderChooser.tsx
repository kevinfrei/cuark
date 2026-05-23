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
import { ReactElement, useState } from 'react';

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

export function FolderChooser(props: {}): ReactElement {
  const locations = ['C:\\', 'D:\\'];
  const favorites = ['Home', 'Downloads', 'Music', 'Pictures', 'Videos'];
  const [curFolder, setCurFolder] = useState<string>('C:\\Users\\freik');
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
  const columns = [
    { colKey: 'filename', label: 'File Name' },
    { colKey: 'size', label: 'Size' },
    { colKey: 'date', label: 'Date Modified' },
    { colKey: 'kind', label: 'Type' },
  ];
  const data = [
    {
      filename: 'MyFile1.txt',
      size: 153,
      date: new Date('Aug 6 1993'),
      kind: 'Text File',
    },
    {
      filename: 'MyFile2.png',
      size: 2468,
      date: new Date('Aug 22 1998'),
      kind: 'PNG Image',
    },
  ];
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
              {favorites.map((fav) => (
                <div key={fav}>{fav}</div>
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
                  <TableCell>{val.filename}</TableCell>
                  <TableCell>{val.size}</TableCell>
                  <TableCell>{val.date.toDateString()}</TableCell>
                  <TableCell>{val.kind}</TableCell>
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
