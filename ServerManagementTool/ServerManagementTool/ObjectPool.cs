using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ServerManagementTool
{
    public interface IPoolingObject
    {
        void Undispose();
    }

    public abstract class ObjectPool<T> : IPoolingObject, IDisposable where T : IPoolingObject, new()
    {
        private static LinkedList<IPoolingObject> Pool = new LinkedList<IPoolingObject>();
        private bool IsDisposed = false;

        public static T New()
        {
            lock( Pool )
            {
                if( Pool.Count > 0 )
                {
                    IPoolingObject Elem = Pool.Last.Value;
                    Pool.RemoveLast();
                    Elem.Undispose();

                    return ( T )Elem;
                }
            }

            return new T();
        }

        public static void AddToPool( IPoolingObject Elem )
        {
            lock( Pool )
                Pool.AddLast( Elem );
        }

        public void Dispose()
        {
            if( false == IsDisposed )
            {
                AddToPool( this );
                IsDisposed = true;
            }
        }

        public void Undispose()
        {
            IsDisposed = false;
            ReInit();
        }

        protected abstract void ReInit();
    }
}
