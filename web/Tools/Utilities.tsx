import {
  ISliderStyles,
  ISpinButtonStyleProps,
  ISpinButtonStyles,
  IStyleFunctionOrObject,
  Position,
  SpinButton,
} from '@fluentui/react';
import { MakeLog } from '@freik/logger';
import {
  chkAnyOf,
  isArrayOfString,
  isNumber,
  isString,
  isUndefined,
} from '@freik/typechk';
import {
  CSSProperties,
  Component,
  ForwardedRef,
  ReactElement,
  SyntheticEvent,
  useEffect,
  useRef,
  useState,
} from 'react';
import { IpcCall, OpenDialogOptions } from '../Shared/CommonTypes';
import { CallMain } from './Ipc';

const { wrn } = MakeLog('EMP:render:Utilities');

// This is a react component to enable the IPC subsystem to talk to the store,
// keep track of which mode we're in, and generally deal with "global" silliness
export function Utilities(props: {
  audioRef: ForwardedRef<HTMLAudioElement>;
}): ReactElement {
  return <></>;
}

export const mySliderStyles: Partial<ISliderStyles> = {
  thumb: {
    borderWidth: 1,
    width: 6,
    height: 10,
    top: -3,
    zIndex: 100,
  },
  line: {
    zIndex: 100,
  },
};

export type StringSpinButtonProps = {
  id?: string;
  className?: string;
  label?: string;
  value: number;
  filter: (val: string) => number | undefined;
  format: (val: number) => string;
  min: number;
  max: number;
  step: number;
  onChange: (newValue?: number) => void;
  style?: CSSProperties;
  labelPosition?: Position;
  styles?: IStyleFunctionOrObject<ISpinButtonStyleProps, ISpinButtonStyles>;
};

export function StringSpinButton({
  id,
  className,
  label,
  value,
  filter,
  format,
  min,
  max,
  step,
  onChange,
  style,
  styles,
  labelPosition,
}: StringSpinButtonProps): ReactElement {
  const onIncrement = (val: string): string | void => {
    const num = filter(val);
    if (isNumber(num)) {
      return format(Math.min(num + step, max));
    }
  };
  const onDecrement = (val: string): string | void => {
    const num = filter(val);
    if (isNumber(num)) {
      return format(Math.max(num - step, min));
    }
  };
  const onValidate = (val: string): string | void => {
    const num = filter(val);
    if (isNumber(num)) {
      return format(Math.max(Math.min(num, max), min));
    }
  };
  const internalChange = (
    event: SyntheticEvent<HTMLElement>,
    newValue?: string,
  ) => {
    const numVal = isUndefined(newValue) ? newValue : filter(newValue);
    onChange(numVal);
  };
  return (
    <SpinButton
      id={id}
      className={className}
      label={label}
      value={format(value)}
      style={style}
      styles={styles}
      labelPosition={labelPosition}
      onChange={internalChange}
      onValidate={onValidate}
      onIncrement={onIncrement}
      onDecrement={onDecrement}
    />
  );
}

type EBProps = { children: React.ReactNode };
type EBState = { hasError: boolean };
export class ErrorBoundary extends Component<EBProps, EBState> {
  constructor(props: EBProps) {
    super(props);
    this.state = { hasError: false };
  }

  static getDerivedStateFromError(error: unknown) {
    // Update state so the next render will show the fallback UI.
    return { hasError: true, error };
  }

  componentDidCatch(error: unknown, errorInfo: unknown) {
    // You can also log the error to an error reporting service
    wrn(error);
    wrn(errorInfo);
  }

  render() {
    if (this.state.hasError) {
      // You can render any custom fallback UI
      return <h4>Something went wrong</h4>;
    }

    return this.props.children;
  }
}

/**
 * @async
 * Shows an Open dialog for the platform you're on. Use this instead of the
 * long-deprecated `remote` electron module.
 *
 * @param options an
 * [OpenDialogOptions](https://www.electronjs.org/docs/latest/api/dialog)
 * instance describing what kind of Open dialog you want to show
 * @returns A promise that resolves to the array of files/folders selected
 */
export async function ShowOpenDialog(
  options: OpenDialogOptions,
): Promise<string[] | void> {
  const res = await CallMain(
    IpcCall.ShowOpenDialog,
    chkAnyOf(isString, isArrayOfString),
    options,
  );
  if (res) {
    console.log('ShowOpenDialog result:', res);
  } else {
    console.log('ShowOpenDialog cancelled or failed');
  }
  return isString(res) ? [res] : res;
}

export const useEffectOnce = (effect: () => void | (() => void)) => {
  const destroyFunc = useRef<void | (() => void)>(() => {});
  const effectCalled = useRef(false);
  const renderAfterCalled = useRef(false);
  const [val, setVal] = useState<number>(0);

  if (effectCalled.current) {
    renderAfterCalled.current = true;
  }

  useEffect(() => {
    // only execute the effect first time around
    if (!effectCalled.current) {
      destroyFunc.current = effect();
      effectCalled.current = true;
    }

    // this forces one render after the effect is run
    setVal((val) => val + 1);

    return () => {
      // if the comp didn't render since the useEffect was called,
      // we know it's the dummy React cycle
      if (!renderAfterCalled.current) {
        return;
      }
      if (destroyFunc.current) {
        destroyFunc.current();
      }
    };
  }, []);
};
