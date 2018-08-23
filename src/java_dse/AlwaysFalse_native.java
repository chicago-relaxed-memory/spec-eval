public class AlwaysFalse_native {
  static {
    System.loadLibrary("AlwaysFalse_native");
  }

  private native int get_native();

  public static boolean get() {
    return new AlwaysFalse_native().get_native() == 1;
  }
}

